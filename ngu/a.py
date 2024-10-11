import pywt  
import numpy as np  
import cv2  

# 读入原始彩色图像和水印彩色图像
original_image = cv2.imread('/home/sweet/ngu/图像/原图像.jpg')  
watermark_image = cv2.imread('/home/sweet/ngu/图像/水印图片.jpg')  # 读取为彩色图像

# 拆分原始图像的通道
b_channel, g_channel, r_channel = cv2.split(original_image)

# 拆分水印图像的通道
b_watermark, g_watermark, r_watermark = cv2.split(watermark_image)

# 定义一个函数对单通道进行水印嵌入
def embed_watermark(channel, watermark_image, alpha=0.1):
    # 执行小波分解
    coeffs = pywt.wavedec2(channel, 'haar', level=2)
    
    # 提取二级高频系数
    cH2, cV2, cD2 = coeffs[2]
    
    # 将水印调整为与高频系数相同的大小
    watermark_resized = cv2.resize(watermark_image, (cH2.shape[1], cH2.shape[0]))
    
    # 在二级水平高频部分嵌入水印
    cH2_with_watermark = cH2 + alpha * watermark_resized
    
    # 更新高频系数
    coeffs[2] = (cH2_with_watermark, cV2, cD2)
    
    # 执行逆小波变换，重构通道图像
    watermarked_channel = pywt.waverec2(coeffs, 'haar')
    
    # 限制像素值在0到255之间
    return np.clip(watermarked_channel, 0, 255)

# 对每个通道分别嵌入对应的水印通道
b_channel_watermarked = embed_watermark(b_channel, b_watermark)
g_channel_watermarked = embed_watermark(g_channel, g_watermark)
r_channel_watermarked = embed_watermark(r_channel, r_watermark)

# 合并处理后的三个通道为一个完整的彩色图像
watermarked_image = cv2.merge([b_channel_watermarked.astype(np.uint8), 
                               g_channel_watermarked.astype(np.uint8), 
                               r_channel_watermarked.astype(np.uint8)])

# 保存带水印的彩色图像
cv2.imwrite('/home/sweet/ngu/图像/通过嵌入彩色水印图片变换后的彩色载体图片.jpg', watermarked_image)
