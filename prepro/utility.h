#pragma once



namespace RapidOCR {

class Utility {
public:
  static std::vector<std::string> ReadDict(const std::string &path);

  static void
  VisualizeBboxes(const cv::Mat &srcimg,
                  const std::vector<std::vector<std::vector<int>>> &boxes);

  template <class ForwardIterator>
  inline static size_t argmax(ForwardIterator first, ForwardIterator last) {
    return std::distance(first, std::max_element(first, last));
  }

  static cv::Mat GetRotateCropImage(const cv::Mat& srcimage, std::vector<std::vector<int>> box);
};

} // namespace RapidOCR