import numpy as np
import cv2
import pywt
import seal

# SEAL 设置
def create_seal_context():
    parms = seal.EncryptionParameters(seal.scheme_type.BFV)
    poly_modulus_degree = 8192
    parms.set_poly_modulus_degree(poly_modulus_degree)
    parms.set_coeff_modulus(seal.CoeffModulus.BFVDefault(poly_modulus_degree))
    parms.set_plain_modulus(seal.PlainModulus.Batching(poly_modulus_degree, 20))

    context = seal.SEALContext.Create(parms)
    return context

def encrypt_with_seal(context, keygen, encryptor, data):
    plain_data = seal.Plaintext()
    plain_data.set_from_vector(data.tolist())
    cipher_text = seal.Ciphertext()
    encryptor.encrypt(plain_data, cipher_text)
    return cipher_text

def decrypt_with_seal(decryptor, cipher_text):
    plain_data = seal.Plaintext()
    decryptor.decrypt(cipher_text, plain_data)
    return np.array(plain_data.to_vector(), dtype=np.uint8)

# 创建 SEAL 上下文
context = create_seal_context()
keygen = seal.KeyGenerator(context)
encryptor = seal.Encryptor(context, keygen.public_key())
decryptor = seal.Decryptor(context, keygen.secret_key())

# 读取载体图像和水印图像
carrier_image = cv2.imread('/home/sweet/git/learngit/ngu/图像/原图像.jpg', cv2.IMREAD_GRAYSCALE)
watermark_image = cv2.imread('/home/sweet/git/learngit/ngu/图像/水印图片.jpg', cv2.IMREAD_GRAYSCALE)

# 进行二重小波分解
coeffs = pywt.wavedec2(carrier_image, 'haar', level=2)

# 对二级高频系数进行解密
CH2_encrypted = coeffs[2][1]  # 加密后的CH2
CH2_decrypted = decrypt_with_seal(decryptor, CH2_encrypted)

# 对解密后的CH2进行同态加密
CH2_homomorphic_encrypted = encrypt_with_seal(context, keygen, encryptor, CH2_decrypted)

# 对水印进行同态加密
watermark_homomorphic_encrypted = encrypt_with_seal(context, keygen, encryptor, watermark_image)

# 将水印嵌入到同态加密后的CH2中
CH2_with_watermark = CH2_homomorphic_encrypted + watermark_homomorphic_encrypted

# 更新小波系数
coeffs[2] = (coeffs[2][0], CH2_with_watermark, coeffs[2][2])

# 进行逆小波变换，生成带水印的图像
watermarked_image = pywt.waverec2(coeffs, 'haar')

# 保存嵌入水印后的图像
cv2.imwrite('/home/sweet/git/learngit/ngu/图像/灰度bfv带水印的图像.jpg', watermarked_image.astype(np.uint8))

# 输出提示信息，表示程序执行完毕
print("水印已成功嵌入并保存。")
