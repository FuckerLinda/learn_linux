#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmssl/sm2.h>
#include <gmssl/sm3.h>
#include <gmssl/sm4.h>
#include <gmssl/rand.h>

void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
    }
    printf("\n");
}

int main() {
    int ret = 0;

    // 1. 生成SM2密钥对
    SM2_KEY sm2_key;
    printf("生成SM2密钥对...\n");
    if (sm2_key_generate(&sm2_key) != 1) {
        fprintf(stderr, "SM2密钥生成失败\n");
        return EXIT_FAILURE;
    }
    printf("SM2密钥生成成功\n");

    // 2. 生成随机SM4对称密钥和IV
    uint8_t sm4_key[16];
    uint8_t iv[SM4_BLOCK_SIZE]; // 新增IV
    if (rand_bytes(sm4_key, sizeof(sm4_key)) != 1 ||
        rand_bytes(iv, sizeof(iv)) != 1) {       // 生成随机IV
        fprintf(stderr, "密钥或IV生成失败\n");
        return EXIT_FAILURE;
    }
    print_hex("SM4对称密钥", sm4_key, sizeof(sm4_key));
    print_hex("IV", iv, sizeof(iv));

    // 3. 使用SM4加密数据（修正参数）
    const char *plaintext = "Hello, GMSSL!";
    size_t plaintext_len = strlen(plaintext);
    uint8_t ciphertext[256];
    size_t ciphertext_len;

    SM4_KEY sm4_enc_key;
    sm4_set_encrypt_key(&sm4_enc_key, sm4_key);
    if (sm4_cbc_padding_encrypt(&sm4_enc_key, iv, (uint8_t *)plaintext, plaintext_len, ciphertext, &ciphertext_len) != 1) {
        fprintf(stderr, "SM4加密失败\n");
        return EXIT_FAILURE;
    }
    print_hex("SM4加密后的密文", ciphertext, ciphertext_len);

    // 4. 使用SM2公钥加密SM4密钥
    uint8_t encrypted_key[128];
    size_t encrypted_key_len;
    if (sm2_encrypt(&sm2_key, sm4_key, sizeof(sm4_key), encrypted_key, &encrypted_key_len) != 1) {
        fprintf(stderr, "SM2加密失败\n");
        return EXIT_FAILURE;
    }
    print_hex("SM2加密后的密钥", encrypted_key, encrypted_key_len);

    // 5. 使用SM2私钥解密SM4密钥
    uint8_t decrypted_key[16];
    size_t decrypted_key_len;
    if (sm2_decrypt(&sm2_key, encrypted_key, encrypted_key_len, decrypted_key, &decrypted_key_len) != 1) {
        fprintf(stderr, "SM2解密失败\n");
        return EXIT_FAILURE;
    }
    print_hex("解密后的SM4密钥", decrypted_key, decrypted_key_len);

    // 6. 使用SM4解密数据（修正参数）
    uint8_t decrypted[256];
    size_t decrypted_len;
    SM4_KEY sm4_dec_key;
    sm4_set_decrypt_key(&sm4_dec_key, decrypted_key);
    if (sm4_cbc_padding_decrypt(&sm4_dec_key, iv, ciphertext, ciphertext_len, decrypted, &decrypted_len) != 1) {
        fprintf(stderr, "SM4解密失败\n");
        return EXIT_FAILURE;
    }
    decrypted[decrypted_len] = '\0';
    printf("解密后的明文: %s\n", decrypted);

    // 7. 哈希验证
    uint8_t hash[SM3_DIGEST_SIZE];
    sm3_digest((uint8_t *)plaintext, plaintext_len, hash);
    print_hex("原始数据SM3哈希", hash, SM3_DIGEST_SIZE);

    uint8_t decrypted_hash[SM3_DIGEST_SIZE];
    sm3_digest(decrypted, decrypted_len, decrypted_hash);
    print_hex("解密数据SM3哈希", decrypted_hash, SM3_DIGEST_SIZE);

    if (memcmp(hash, decrypted_hash, SM3_DIGEST_SIZE) == 0) {
        printf("哈希验证成功\n");
    } else {
        printf("哈希验证失败\n");
    }

    return EXIT_SUCCESS;
}
