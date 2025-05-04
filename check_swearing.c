#include <stdio.h>
#include <string.h>
#define MAXN 1000010

// 定义脏话数组
char swear_words[7][10] = {"装逼", "操你妈", "他妈的", "你妈的", "fuck", "shit", "asshole"};
int kmp[MAXN];
int j;
char a[MAXN];

// 检查字符串 b 是否是字符串 a 的子串
int check_swearing(const char* a, const char* b) {
    int la = strlen(a);
    int lb = strlen(b);

    // 计算 KMP 的 next 数组
    j = 0;
    for (int i = 1; i < lb; i++) {
        while (j > 0 && b[i] != b[j]) {
            j = kmp[j - 1];
        }
        if (b[j] == b[i]) {
            j++;
        }
        kmp[i] = j;
    }

    j = 0;
    for (int i = 0; i < la; i++) {
        while (j > 0 && b[j] != a[i]) {
            j = kmp[j - 1];
        }
        if (b[j] == a[i]) {
            j++;
        }
        if (j == lb) {
            return 0; // 找到脏话，返回 0
        }
    }
    return 1; // 未找到脏话，返回 1
}

// 查找输入字符串中是否包含脏话
int find_swearing() {
    int found = 0;
    // 修正循环范围，遍历所有脏话
    for (int i = 0; i < 7; i++) {
        if (check_swearing(a, swear_words[i]) == 0) {
            found = 1;
            break;
        }
    }
    // 返回检测结果
    return found;
}

int main() {
    // 读取字符串 a
    printf("消息: ");
    fgets(a, MAXN, stdin);
    a[strcspn(a, "\n")] = 0;  // 去除换行符

    if (find_swearing()) {
        printf("问题消息，不予发送\n");
    } else {
        printf("消息可发送\n");
    }

    return 0;
}    