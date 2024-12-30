import numpy as np
from seal import EncryptionParameters, SEALContext, KeyGenerator, Encryptor, Decryptor, Evaluator, BatchEncoder, scheme_type
from PIL import Image

def encrypt_image(image_path, encryptor, encoder):
    """
    将图像转换为数值矩阵并进行BGV同态加密
    :param image_path: 图像文件的路径
    :param encryptor: SEAL库中的加密器对象
    :param encoder: SEAL库中的BatchEncoder对象，用于将像素数据编码为明文
    :return: 加密后的像素数据
    """
    # 打开图像并转换为灰度图（每个像素一个值）
    img = Image.open(image_path).convert('L')
    img_array = np.array(img)

    # 将图像转换为一维列表（BGV加密需要处理成数值列表）
    pixel_values = img_array.flatten().tolist()

    # 编码为明文
    plain = encoder.encode(pixel_values)

    # 对每个像素进行加密
    encrypted_pixels = encryptor.encrypt(plain)

    return encrypted_pixels

# 假设你已经初始化了 SEAL 上下文，创建了加密器、编码器等对象
# 示例：初始化加密参数
def initialize_seal():
    # 设置加密参数
    parms = EncryptionParameters(scheme_type.bgv)
    parms.set_poly_modulus_degree(8192)
    parms.set_coeff_modulus(EncryptionParameters.coeff_modulus_128(8192))
    parms.set_plain_modulus(256)

    # 创建SEAL上下文
    context = SEALContext(parms)

    # 生成密钥
    keygen = KeyGenerator(context)
    public_key = keygen.public_key()
    secret_key = keygen.secret_key()

    encryptor = Encryptor(context, public_key)
    decryptor = Decryptor(context, secret_key)
    encoder = BatchEncoder(context)

    return encryptor, decryptor, encoder

# 初始化
encryptor, decryptor, encoder = initialize_seal()

# 加密图片
encrypted_image = encrypt_image("/home/sweet/git/learngit/ngu/图像/原图像.jpg", encryptor, encoder)

# 检查加密结果的类型
print(f"加密后的数据类型是: {type(encrypted_image)}")
