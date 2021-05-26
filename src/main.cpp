#include <cstdio>
#include <string>
#include <unistd.h>
#include <fstream>
#include <json.h>

const char *project = "https://github.com/chiro2001/onekey-proxy",
        *author = "Chiro",
        *email = "chiro2001@163.com",
        *version = "0.0.1",
        *help_str = "Usage: %s [-s]/[-e] [-h] [-f selected_file_name] [-p proxy] [-o overrides]\n"
                    "\n"
                    "Examples:\n"
                    "\t%s -h\n"
                    "\t%s -s\n"
                    "\t%s -s -p http://127.0.0.1:8081/\n"
                    "\t%s -s -p http://localhost:8081/ -o 10.*;127.0.0.*; -v\n"
                    "\t%s -s -p http://localhost:8081/ -s -f new_config.json\n"
                    "\t%s -e\n"
                    "\n"
                    "Description:\n"
                    "\t-h\t\tShow this help info and exit.\n"
                    "\t-s\t\tSet proxy on.\n"
                    "\t-e\t\tSet proxy off.\n"
                    "\t-v\t\tSave config data to json file. (Default %s.)\n"
                    "\t-p proxy\tSet proxy address.\n"
                    "\t-o overrides\tSet overrides.\n"
                    "\n"
                    "Author: %s %s\n"
                    "Project: %s\n"
                    "Version: %s " __DATE__ " " __TIME__;
const char *filename_default = "onekey-proxy.json";

enum class operation {
  start = 1u,
  end = 1u << 1u,
  help = 1u << 2u,
  file = 1u << 3u,
  save = 1u << 4u
};

void print_help(int argc, char *argv[]) {
  std::string self = "onekey-proxy";
  if (argc >= 1) {
    self = std::string(argv[0]);
    if (self.find_last_of('/') != -1)
      self = self.substr(self.find_last_of('/') + 1);
    if (self.find_last_of('\\') != -1)
      self = self.substr(self.find_last_of('\\') + 1);
  }
  printf(help_str, self.c_str(), self.c_str(), self.c_str(), self.c_str(), self.c_str(), self.c_str(), self.c_str(),
         filename_default, author, email, project, version);
}

class Args {
public:
  // unsigned int mode = static_cast<int>(operation::start);
  unsigned int mode = 0;
  std::string proxy, overrides, filename = filename_default;
};

bool parse_args(int argc, char **argv, Args &args) {
  int opt;
  // 参数控制，start, end, help, proxy, override
  const char *optstring = "sehvf:p:o:";
  if (argc <= 1) return false;
  while ((opt = getopt(argc, argv, optstring)) != -1) {
    if (opt == '?') return false;
    switch (opt) {
      case 's':
        args.mode |= static_cast<unsigned int>(operation::start);
        break;
      case 'e':
        args.mode |= static_cast<unsigned int>(operation::end);
        break;
      case 'h':
        args.mode |= static_cast<unsigned int>(operation::help);
        return true;
      case 'v':
        // 保存到配置文件
        args.mode |= static_cast<unsigned int>(operation::save);
        break;
      case 'p':
        args.proxy = std::string(optarg);
        break;
      case 'o':
        args.overrides = std::string(optarg);
        break;
      case 'f':
        args.filename = std::string(optarg);
        break;
      default:
        return false;
    }
    // printf("opt = %c\n", opt);  // 命令参数，亦即 -a -b -c -d
    // printf("optarg = %s\n", optarg); // 参数内容
    // printf("optind = %d\n", optind); // 下一个被处理的下标值
    // printf("argv[optind - 1] = %s\n\n", argv[optind - 1]); // 参数内容
  }
  if ((args.mode & static_cast<unsigned int>(operation::start) &&
       args.mode & static_cast<unsigned int>(operation::end)) ||
      (args.mode & static_cast<unsigned int>(operation::save) &&
       args.mode & static_cast<unsigned int>(operation::end)))
    return false;
  return true;
}

inline std::ifstream file_get(const std::string &name) {
  std::ifstream f(name.c_str());
  if (!f.good()) throw std::runtime_error(std::string("File " + name + " not found!").c_str());
  return f;
}

void save_data(const Args &args, const std::string &filename) {
  Json::Value root;
  root["overrides"] = args.overrides;
  root["proxy"] = args.proxy;
  std::ofstream ofs(filename);
  Json::StreamWriterBuilder builder;
  std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  writer->write(root, &ofs);
  ofs.close();
}

void load_data(Args &args, const std::string &filename) {
  auto ifs = file_get(filename);
  Json::CharReaderBuilder readerBuilder;
  Json::Value root;
  JSONCPP_STRING errs;
  if (!Json::parseFromStream(readerBuilder, ifs, &root, &errs))
    throw std::runtime_error(std::string("Cannot parse json: " + errs));
  args.overrides = root["overrides"].asString();
  args.proxy = root["proxy"].asString();
  ifs.close();
}

void set_proxy_enable(bool enable = true) {
  printf("%s proxy...", enable ? "Starting" : "Stopping");
  system(std::string(
          "reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\" "
          "/v ProxyEnable /t REG_DWORD /d " +
          std::string(enable ? "1" : "0") + " /f").c_str());
}

void set_proxy(const std::string &proxy) {
  printf("Setting proxy to %s...", proxy.c_str());
  system(std::string("reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\" "
                     "/v ProxyServer /d \"" + proxy + "\" /f").c_str());
}

void set_overrides(const std::string &overrides) {
  printf("Setting overrides to %s...", overrides.c_str());
  system(std::string("reg add \"HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\" "
                     "/v ProxyOverride /t REG_SZ /d \"" + overrides + "\" /f").c_str());
}

int main(int argc, char *argv[]) {
  Args args;
  if (!parse_args(argc, argv, args)) {
    // 打印帮助字符串
    print_help(argc, argv);
    return 1;
  }
  if (args.mode == 0) args.mode |= static_cast<unsigned int>(operation::start);
  if (args.mode & static_cast<unsigned int>(operation::help)) {
    print_help(argc, argv);
    return 0;
  }
  if (args.proxy.empty() && args.overrides.empty())
    try {
      load_data(args, args.filename);
    } catch (std::runtime_error &e) {
      fprintf(stderr, "%s\n", e.what());
      print_help(argc, argv);
      return 1;
    }
  if (args.mode & static_cast<unsigned int>(operation::start)) {
    // if (args.proxy.empty() && args.overrides.empty()) {
    //   print_help(argc, argv);
    //   return 1;
    // }
    set_proxy_enable(true);
    if (!args.proxy.empty()) set_proxy(args.proxy);
    if (!args.overrides.empty()) set_overrides(args.overrides);
  }
  if (args.mode & static_cast<unsigned int>(operation::end)) {
    set_proxy_enable(false);
  }
  if (args.mode & static_cast<unsigned int>(operation::save)) {
    save_data(args, args.filename);
    printf("Saving data to %s...\n", args.filename.c_str());
    return 0;
  }
  return 0;
}