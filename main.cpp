#include <Magick++.h>
#include <iostream>
#include <optional>
#include <filesystem>

namespace {

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
// NOLINTEND

} // namespace

auto main(int argc, char *argv[]) -> int {
    Magick::InitializeMagick(*argv);

    std::cout << "\x1b[38;2;255;100;0mTRUECOLOR\x1b[0m\n";

    const std::optional<std::filesystem::path> content = getImageFromCmdlineArgs(argc, argv);
    if (content.has_value()) {
        std::cout << "parsed " << content.value() << '\n';
    } else {
        std::cout << "parsed nothing\n";
    }

    // handle cmdline args
    //
    // should take in an image path from cmdline
    //
    // then go and do the render

    return 0;
}
