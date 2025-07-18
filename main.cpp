#include <Magick++.h>
#include <iostream>
#include <optional>
#include <filesystem>
#include <format>

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

void printImageArt(const std::filesystem::path &imagePath, const uint32_t termWidth, const uint32_t termHeight) {
  const Magick::Image image(imagePath.c_str());
  const Magick::Geometry size = image.size();

  for (uint32_t j = 0; j < termHeight; j++) {
    for (uint32_t i = 0; i < termWidth; i++) {
      const float percentX = static_cast<float>(i) / static_cast<float>(termWidth);
      const float percentY = static_cast<float>(j) / static_cast<float>(termHeight);

      const auto imgX = static_cast<uint32_t>(percentX * static_cast<float>(size.width()));
      const auto imgY = static_cast<uint32_t>(percentY * static_cast<float>(size.height()));

      const Magick::Color color = image.pixelColor(imgX, imgY);
      constexpr float MAGICK_QUANTUM_RANGE = 65535.0;
      const auto red  = static_cast<uint8_t>(color.quantumRed() * 255 / MAGICK_QUANTUM_RANGE);
      const auto green  = static_cast<uint8_t>(color.quantumGreen() * 255 / MAGICK_QUANTUM_RANGE);
      const auto blue  = static_cast<uint8_t>(color.quantumBlue() * 255 / MAGICK_QUANTUM_RANGE);

      // Good Glyphs
      // - █
      // - 8
      // - #
      constexpr std::string_view GLYPH = "#";
      const std::string colorText =
        std::format("\x1b[38;2;{};{};{}m{}\x1b[0m", red, green, blue, GLYPH);

       std::cout << colorText;
    }
    std::cout << "\n";
  }
}

} // namespace

auto main(int argc, char *argv[]) -> int {

    std::cout << "\x1b[38;20;255;100;0mTRUECOLOR\x1b[0m\n";

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
      constexpr uint32_t WIDTH = 160;
      constexpr uint32_t HEIGHT = 45;
      Magick::InitializeMagick(*argv);
      printImageArt(path, WIDTH, HEIGHT);
    } catch (std::exception &e) {
      std::cout << "Ran into exception! " << e.what() << "\n";
    }

    return 0;
}
