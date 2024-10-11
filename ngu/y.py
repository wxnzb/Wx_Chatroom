import pywt  
import numpy as np  
import cv2  

# 读入原始图像和水印图像  
original_image = cv2.imread('/home/sweet/ngu/图像/原图像.jpg', cv2.IMREAD_GRAYSCALE)  
watermark_image = cv2.imread('/home/sweet/ngu/图像/水印图片.jpg', cv2.IMREAD_GRAYSCALE)  

# 确保水印图像小于或等于原始图像  
assert watermark_image.shape[0] <= original_image.shape[0]  
assert watermark_image.shape[1] <= original_image.shape[1]  

# 执行双重小波分解  
coeffs = pywt.wavedec2(original_image, 'haar', level=2)  

# 提取二级高频部分  
cH2, cV2, cD2 = coeffs[2]  # 这里coeffs[2]指的是二级分解后的高频系数  

# 将水印嵌入二级高频ch2  
# 注意，水印需要进行某种形式的调整以适合嵌入位置  
watermark_resized = cv2.resize(watermark_image, (cH2.shape[1], cH2.shape[0]))  

# 嵌入水印，通常使用加法或者其它方法  
alpha = 0.111  # 控制水印强度的参数  
cH2_with_watermark = cH2 + alpha * watermark_resized  

# 更新系数  
coeffs[2] = (cH2_with_watermark, cV2, cD2)  

# 重构图像  
watermarked_image = pywt.waverec2(coeffs, 'haar')  

# 防止逼近的像素值超出范围  
watermarked_image = np.clip(watermarked_image, 0, 255)  

# 保存水印图像  
cv2.imwrite('/home/sweet/ngu/图像/变换后的载体图片.jpg', watermarked_image)