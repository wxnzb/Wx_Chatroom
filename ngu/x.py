import cv2  
import numpy as np  
import pywt  
import matplotlib.pyplot as plt  

# 加载图像  
def load_image(filename):  
    return cv2.imread(filename)  

# 显示图像  
def show_image(img, title='Image'):  
    plt.imshow(img, cmap='gray')  
    plt.title(title)  
    plt.axis('off')  
    plt.show()  

# 灰度化  
def grayscale(img):  
    return cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)  

# 归一化  
def normalize(img):  
    return cv2.normalize(img, None, alpha=0, beta=255, norm_type=cv2.NORM_MINMAX, dtype=cv2.CV_8U)  

# 小波变换  
def discrete_wavelet_transform(img):  
    coeffs = pywt.wavedec2(img, 'haar', level=2)  # 使用Haar小波进行二级分解  
    return coeffs  

# 嵌入水印 (在二级水平高频系数中嵌入水印)  
def embed_watermark(coeffs, watermark):  
    cA1 = coeffs[0]  # 第一级低频系数  
    cH1, cV1, cD1 = coeffs[1]  # 第一级高频系数  

    if len(coeffs) > 2:  # 确保第二级存在  
        cA2 = coeffs[2]  # 第二级低频系数  
        if len(coeffs) > 3:  # 确保第二级高频系数存在  
            cH2, cV2, cD2 = coeffs[3]  # 第二级高频系数  

            # 只在二级水平高频系数中嵌入水印  
            cH2 = cH2 + watermark  # 在二级水平高频系数中嵌入水印  

            return [cA1, (cH1, cV1, cD1), (cA2, (cH2, cV2, cD2))]  # 返回修改后的系数  
    return coeffs  # 如果没有足够的系数，返回未修改的系数  

# 主函数：嵌入水印并保存图像  
def main():  
    input_filename = '/home/sweet/ngu/图像/原图像.jpg'  # 输入文件名  
    output_filename = '/home/sweet/ngu/图像/结果图像.jpg'  # 输出文件名  

    # 加载图像  
    img = load_image(input_filename)  
    gray_img = grayscale(img)  # 灰度化  
    norm_img = normalize(gray_img)  # 归一化  

    # 生成与二级高频系数大小一致的水印矩阵  
    coeffs = discrete_wavelet_transform(norm_img)  
    
    #----------------------------  
    print("Number of coefficient sets:", len(coeffs))  
    print("Coeffs structure:", coeffs)  
    #----------------------------   

    if len(coeffs) > 3:  
        cH2, cV2, cD2 = coeffs[3]  # 获取二级高频系数  
        watermark = np.random.randn(*cH2.shape) * 0.1  # 生成与二级高频系数大小一致的水印  

        # 在二级高频系数中嵌入水印  
        coeffs_with_watermark = embed_watermark(coeffs, watermark)  

        # 逆小波变换重构图像  
        reconstructed_img_with_watermark = pywt.waverec2(coeffs_with_watermark, 'haar')  
        reconstructed_img_with_watermark = np.clip(reconstructed_img_with_watermark, 0, 255).astype(np.uint8)  # 修正像素范围  

        # 显示并保存带水印的图像  
        show_image(reconstructed_img_with_watermark, title='Reconstructed Image with Watermark')  
        cv2.imwrite(output_filename, reconstructed_img_with_watermark)  # 保存结果图像  
    else:  
        print("Not enough coefficient sets to embed watermark.")  

if __name__ == '__main__':  
    main()