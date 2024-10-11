import pywt  
import numpy as np  
import cv2  

# 读入原始彩色图像
original_image = cv2.imread('/home/sweet/ngu/图像/原图像.jpg')  
watermark_image = cv2.imread('/home/sweet/ngu/图像/水印图片.jpg', cv2.IMREAD_GRAYSCALE)  

# 拆分彩色图像的通道
b_channel, g_channel, r_channel = cv2.split(original_image)

# 定义一个函数对单通道进行水印嵌入
def embed_watermark(channel, watermark_image, alpha=0.1):
    coeffs = pywt.wavedec2(channel, 'haar', level=2)
    cH2, cV2, cD2 = coeffs[2]
    watermark_resized = cv2.resize(watermark_image, (cH2.shape[1], cH2.shape[0]))
    cH2_with_watermark = cH2 + alpha * watermark_resized
    coeffs[2] = (cH2_with_watermark, cV2, cD2)
    watermarked_channel = pywt.waverec2(coeffs, 'haar')
    return np.clip(watermarked_channel, 0, 255)

# 对每个通道分别嵌入水印
b_channel_watermarked = embed_watermark(b_channel, watermark_image)
g_channel_watermarked = embed_watermark(g_channel, watermark_image)
r_channel_watermarked = embed_watermark(r_channel, watermark_image)

# 合并处理后的通道
watermarked_image = cv2.merge([b_channel_watermarked.astype(np.uint8), 
                               g_channel_watermarked.astype(np.uint8), 
                               r_channel_watermarked.astype(np.uint8)])

# 保存结果
cv2.imwrite('/home/sweet/ngu/图像/通过嵌入灰度水印图片变换后的彩色载体图片.jpg', watermarked_image)
