import pywt
import numpy as np
import cv2
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
from Crypto.Random import get_random_bytes

# 初始化密钥和加密函数
key = get_random_bytes(16)  # AES 密钥
iv = get_random_bytes(16)  # 使用随机的初始向量 (IV)

# 加密函数
def encrypt_image(image):
    flat_image = image.flatten()  # 将图像展平为一维
    flat_image_bytes = flat_image.tobytes()  # 转换为字节流
    padded_image = pad(flat_image_bytes, AES.block_size)  # 使用pad进行填充
    cipher = AES.new(key, AES.MODE_CBC, iv)  # 每次加密都使用新的AES实例
    encrypted_image = cipher.encrypt(padded_image)  # 加密
    return encrypted_image

# 解密函数
def decrypt_image(encrypted_image, iv, original_shape):
    cipher = AES.new(key, AES.MODE_CBC, iv=iv)  # 使用原始IV
    decrypted_padded = cipher.decrypt(encrypted_image)  # 解密
    decrypted_image_bytes = unpad(decrypted_padded, AES.block_size)  # 去除填充
    decrypted_image_np = np.frombuffer(decrypted_image_bytes, dtype=np.uint8)  # 转换回numpy数组
    return decrypted_image_np.reshape(original_shape)  # 重新构建成图像的形状

# 读入原始彩色图像
original_image = cv2.imread('/home/sweet/ngu/图像/原图像.jpg')
watermark_image = cv2.imread('/home/sweet/ngu/图像/水印图片.jpg', cv2.IMREAD_GRAYSCALE)

# 确保水印图像正确读取
if watermark_image is None:
    raise ValueError("水印图片未能正确读取，请检查路径。")

# 拆分彩色图像的通道
b_channel, g_channel, r_channel = cv2.split(original_image)

# 定义一个函数对单通道进行水印嵌入，先加密cH2，再嵌入加密的水印
def embed_encrypted_watermark(channel, watermark_image, alpha=0.1):
    coeffs = pywt.wavedec2(channel, 'haar', level=2)
    cH2, cV2, cD2 = coeffs[2]

    # 将cH2归一化到[0, 255]
    cH2_normalized = ((cH2 - np.min(cH2)) / (np.max(cH2) - np.min(cH2)) * 255).astype(np.uint8)

    # 对cH2进行同态加密
    cH2_encrypted = encrypt_image(cH2_normalized)

    # 将水印调整为与高频系数相同的大小
    watermark_resized = cv2.resize(watermark_image, (cH2.shape[1], cH2.shape[0]))

    # 解密cH2
    decrypted_cH2 = decrypt_image(cH2_encrypted, iv, cH2_normalized.shape)

    # 嵌入加密后的水印
    combined_with_watermark = np.clip(decrypted_cH2 + alpha * watermark_resized.astype(np.float32), 0, 255)

    # 更新小波系数
    coeffs[2] = (combined_with_watermark, cV2, cD2)

    # 执行逆小波变换，重构通道图像
    watermarked_channel = pywt.waverec2(coeffs, 'haar')

    # 限制像素值在0到255之间
    return np.clip(watermarked_channel, 0, 255)

# 对每个通道分别嵌入加密的水印
b_channel_watermarked = embed_encrypted_watermark(b_channel, watermark_image)
g_channel_watermarked = embed_encrypted_watermark(g_channel, watermark_image)
r_channel_watermarked = embed_encrypted_watermark(r_channel, watermark_image)

# 合并处理后的通道
watermarked_image = cv2.merge([b_channel_watermarked.astype(np.uint8),
                               g_channel_watermarked.astype(np.uint8),
                               r_channel_watermarked.astype(np.uint8)])

# 保存结果
cv2.imwrite('/home/sweet/ngu/图像/通过加密嵌入灰度水印后的彩色载体图片.jpg', watermarked_image)
