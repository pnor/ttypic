#include <Magick++.h>
#include <iostream>
#include <optional>
#include <filesystem>

namespace {

// ===================== Command Line Parsing =====================

void helpMessage(){std::cout << "usage: ttypic [image-path]\n"; }

// NOLINTBEGIN
// Don't lint the pointer arithmetic involved with argv

std::optional<std::filesystem::path> getImageFromCmdlineArgs(const int argc,
                                                             char *argv[]) {
  if (argc <= 1) {
    return std::nullopt;
  }
  return argv[1] != nullptr ? std::make_optional(std::filesystem::path(
                                  std::string_view(argv[1])))
                            : std::nullopt;
}

void handleHelpFlagIfPresent(const int argc, char *argv[]) {
  if (argc <= 1 || argv[1] == nullptr) {
    return;
  }

  const std::string_view firstArg = argv[1];

  if (firstArg == "-h" || firstArg == "--help") {
      helpMessage();
      exit(0);
  }
}
// NOLINTEND

// ===== Image Processing ========================

void printImageArt(const std::filesystem::path &imagePath) {
  const Magick::Image image(imagePath.c_str());
  const Magick::Geometry size = image.size();
  std::cout << "image width = " << size.width() << "\n";
  std::cout << "image height = " << size.height() << "\n";
}

} // namespace

auto main(int argc, char *argv[]) -> int {

    std::cout << "\x1b[38;2;255;100;0mTRUECOLOR\x1b[0m\n";

    handleHelpFlagIfPresent(argc, argv);

    const std::optional<std::filesystem::path> content = getImageFromCmdlineArgs(argc, argv);

    if (!content.has_value()) {
        helpMessage();
        exit(0);
    }

    const std::filesystem::path& path = content.value();
    if (!std::filesystem::exists(path)) {
      std::cout <<  path.c_str() << " does not exist!\n";
      exit(0);
    }

    try {
      Magick::InitializeMagick(*argv);
      printImageArt(path);
    } catch (std::exception &e) {
      std::cout << "Ran into exception! " << e.what() << "\n";
    }

    return 0;
}
