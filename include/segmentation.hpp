
/**
 * @file segmentation.hpp
 * @author 肖书奇 (xiaosq2000@gmail.com)
 * @brief 菌丝与絮体的图像分割
 * @version 1.0
 * @date 2021-06-13
 * 
 */

#ifndef __SEGMENTATION
#define __SEGMENTATION

#include <iostream>
#include <string>
#include <filesystem>
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

class Segmentation
{
private:
    std::string src_directory_;
    std::string dst_directory_;
    std::vector<std::string> imgs_filename_;

    std::vector<cv::Mat> imgs_src_;
    std::vector<cv::Mat> imgs_src_gray_;
    std::vector<cv::Mat> imgs_pre_segmentation_;
    std::vector<cv::Mat> imgs_src_enhanced_;
    std::vector<cv::Mat> imgs_segmentation_;
    std::vector<cv::Mat> imgs_segmentation_0_;
    std::vector<cv::Mat> imgs_segmentation_1_;

public:
    Segmentation(std::string src_directory = "../share/src/demo", std::string dst_directory = "../share/dst/demo");
    ~Segmentation();
};

Segmentation::Segmentation(std::string src_directory, std::string dst_directory)
{
    this->src_directory_ = src_directory;
    this->dst_directory_ = dst_directory;
    std::filesystem::path p = this->dst_directory_;
    std::filesystem::create_directory(p / "0");
    std::filesystem::create_directory(p / "1");

    std::cout << "Loading src images from " + this->src_directory_ << std::endl;
    cv::Mat img_src;
    for (const auto &entry : std::filesystem::directory_iterator(this->src_directory_))
    {
        if (entry.path().extension() == ".jpg")
        {
            // std::cout << entry.path().string() << std::endl;
            img_src = cv::imread(entry.path().string());
            // cv::medianBlur(img_src, img_src, 11);
            this->imgs_src_.emplace_back(img_src);
            this->imgs_src_enhanced_.emplace_back(img_src);
            this->imgs_src_gray_.emplace_back(cv::Mat::zeros(img_src.size(), img_src.type()));
            this->imgs_pre_segmentation_.emplace_back(cv::Mat::zeros(img_src.size(), img_src.type()));
            this->imgs_segmentation_.emplace_back(cv::Mat::zeros(img_src.size(), img_src.type()));
            this->imgs_segmentation_0_.emplace_back(cv::Mat::zeros(img_src.size(), img_src.type()));
            this->imgs_segmentation_1_.emplace_back(cv::Mat::zeros(img_src.size(), img_src.type()));
            this->imgs_filename_.emplace_back(entry.path().filename().string());
        }
    }
    for (size_t i = 0; i < this->imgs_src_.size(); i++)
    {
        std::cout << "Processing (" + std::to_string(i + 1) + "/" + std::to_string(this->imgs_src_.size()) + ")" << std::endl;
        // 彩色图像转灰度图像
        cv::cvtColor(this->imgs_src_[i], this->imgs_src_gray_[i], cv::COLOR_RGB2GRAY);
        // 自适应阈值分割
        cv::adaptiveThreshold(this->imgs_src_gray_[i], this->imgs_pre_segmentation_[i], 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 101, 4);
        // 彩色图像增强
        for (size_t j = 0; j < this->imgs_src_[i].rows; j++)
        {
            for (size_t k = 0; k < this->imgs_src_[i].cols; k++)
            {
                if (this->imgs_pre_segmentation_[i].at<uchar>(j, k) == 255)
                {
                    this->imgs_src_enhanced_[i].at<cv::Vec3b>(j, k)[0] = 255;
                    this->imgs_src_enhanced_[i].at<cv::Vec3b>(j, k)[1] = 0;
                    this->imgs_src_enhanced_[i].at<cv::Vec3b>(j, k)[2] = 0;
                }
            }
        }
        // 彩色图像转灰度图像
        cv::cvtColor(this->imgs_src_enhanced_[i], this->imgs_src_gray_[i], cv::COLOR_RGB2GRAY);
        // OTSU法阈值分割
        cv::threshold(this->imgs_src_gray_[i], this->imgs_segmentation_[i], 0, 255, cv::THRESH_OTSU);
        this->imgs_segmentation_[i] = 255 - this->imgs_segmentation_[i]; // 黑色背景
        // 开运算：去除噪点
        cv::morphologyEx(this->imgs_segmentation_[i], this->imgs_segmentation_[i], cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7), cv::Point(-1, -1)));
        // 开运算：去除菌丝
        cv::morphologyEx(this->imgs_segmentation_[i], this->imgs_segmentation_0_[i], cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(31, 31), cv::Point(-1, -1)));
        // 图像相减：得到菌丝
        cv::subtract(this->imgs_segmentation_[i], this->imgs_segmentation_0_[i], this->imgs_segmentation_1_[i]);
        // 闭运算：修补断裂的菌丝
        cv::morphologyEx(this->imgs_segmentation_1_[i], this->imgs_segmentation_1_[i], cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(9, 9), cv::Point(-1, -1)));
        // 腐蚀：去除噪点
        cv::erode(this->imgs_segmentation_1_[i], this->imgs_segmentation_1_[i], cv::Mat::eye(cv::Size(5, 5), CV_8UC1));
        // 开运算：菌丝变细
        cv::morphologyEx(this->imgs_segmentation_1_[i], this->imgs_segmentation_1_[i], cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(-1, -1)));
        // 中值滤波：去除噪点
        cv::medianBlur(this->imgs_segmentation_1_[i], this->imgs_segmentation_1_[i], 3);
    }
    std::cout << "Saving dst images to " + this->dst_directory_ << std::endl;

    for (size_t i = 0; i < imgs_src_.size(); i++)
    {
        // 保存结果，黑色背景
        // cv::imwrite(this->dst_directory_ + "/0/" + this->imgs_filename_[i], 255 - this->imgs_segmentation_0_[i]);
        // cv::imwrite(this->dst_directory_ + "/1/" + this->imgs_filename_[i], 255 - this->imgs_segmentation_1_[i]);

        // 保存结果，白色背景
        cv::imwrite(this->dst_directory_ + "/0/" + this->imgs_filename_[i], 255 - this->imgs_segmentation_0_[i]);
        cv::imwrite(this->dst_directory_ + "/1/" + this->imgs_filename_[i], 255 - this->imgs_segmentation_1_[i]);
    }
    std::cout << "Done" << std::endl;
}

Segmentation::~Segmentation()
{
}

#endif
