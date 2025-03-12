/* stuinfo.c - 学生信息管理系统完整代码 */
#include "mysql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 学生信息结构体（对应数据库表结构）
struct student {
    int id;
    char name[20];
    char sex[6];
    char mailbox[40];
    char studep[100];
};

/*------------ 数据库初始化与连接 ------------*/
MYSQL* initial(MYSQL *con) {
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "MySQL库初始化失败\n");
        return NULL;
    }
    con = mysql_init(NULL);
    return con;
}

MYSQL* myconnect(MYSQL *con, char *db) {
    if (!mysql_real_connect(con, "localhost", "root", "root", db, 0, NULL, 0)) {
        fprintf(stderr, "数据库连接失败: %s\n", mysql_error(con));
        return NULL;
    }
    return con;
}

/*------------ 数据库操作函数 ------------*/
void createDB(MYSQL *con, char *cmd) {
    if (mysql_query(con, cmd)) {
        fprintf(stderr, "创建数据库失败: %s\n", mysql_error(con));
    } else {
        printf("数据库创建成功\n");
    }
}

void createTable(MYSQL *con, char *db, char *cmd) {
    if (mysql_select_db(con, db)) {
        fprintf(stderr, "选择数据库失败: %s\n", mysql_error(con));
        return;
    }
    if (mysql_query(con, cmd)) {
        fprintf(stderr, "创建数据表失败: %s\n", mysql_error(con));
    } else {
        printf("数据表创建成功\n");
    }
}

/*------------ 数据查询与展示 ------------*/
void displayTable(MYSQL *con, char *query) {
    // 新增：执行查询语句
    if (mysql_query(con, query)) {
        fprintf(stderr, "查询失败: %s\n", mysql_error(con));
        return;
    }

    MYSQL_RES *result = mysql_store_result(con);
    if (!result) {
        fprintf(stderr, "获取结果集失败: %s\n", mysql_error(con));
        return;
    }

    int num_fields = mysql_num_fields(result);
    MYSQL_ROW row;

    printf("\n----- 查询结果 -----\n");
    while ((row = mysql_fetch_row(result))) {
        for (int i = 0; i < num_fields; i++) {
            printf("%s\t", row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }
    mysql_free_result(result);
}

/*------------ 数据插入与删除 ------------*/
void insertData(MYSQL *con) {
    struct student s;
    char query[1024];

    printf("输入学号: ");
    scanf("%d", &s.id);
    printf("输入姓名: ");
    scanf("%s", s.name);
    printf("输入性别（male/female）: ");
    scanf("%s", s.sex);
    printf("输入邮箱: ");
    scanf("%s", s.mailbox);
    printf("输入院系: ");
    scanf("%s", s.studep);

    sprintf(query, "INSERT INTO stuinfo VALUES(%d, '%s', '%s', '%s', '%s')", 
            s.id, s.name, s.sex, s.mailbox, s.studep);

    if (mysql_query(con, query)) {
        mysql_rollback(con);
        fprintf(stderr, "插入失败: %s\n", mysql_error(con));
    } else {
        mysql_commit(con);
        displayTable(con, "SELECT * FROM stuinfo");
    }
}

void deleteData(MYSQL *con) {
    int id;
    char query[64];
    printf("输入要删除的学号: ");
    scanf("%d", &id);

    sprintf(query, "DELETE FROM stuinfo WHERE id=%d", id);
    if (mysql_query(con, query)) {
        mysql_rollback(con);
        fprintf(stderr, "删除失败: %s\n", mysql_error(con));
    } else {
        mysql_commit(con);
        printf("删除成功\n");
        displayTable(con, "SELECT * FROM stuinfo");
    }
}

/*------------ 菜单驱动逻辑 ------------*/
int menu() {
    int choice;
    printf("\n===== 学生信息管理系统 =====\n");
    printf("1. 创建数据库\n");
    printf("2. 创建数据表\n");
    printf("3. 插入数据\n");
    printf("4. 删除数据\n");
    printf("0. 退出\n");
    printf("请输入选项: ");
    scanf("%d", &choice);
    return choice;
}

int main() {
    MYSQL *con = initial(NULL);
    if (!con) exit(1);

    if (!myconnect(con, "")) exit(1);

    int choice;
    while ((choice = menu()) != 0) {
        switch (choice) {
            case 1:
                createDB(con, "CREATE DATABASE student");
                break;
            case 2:
                createTable(con, "student", 
                    "CREATE TABLE stuinfo(id INT, stuname TEXT, gender TEXT, mailbox TEXT, studep TEXT)");
                break;
            case 3:
                insertData(con);
                break;
            case 4:
                deleteData(con);
                break;
            default:
                printf("无效选项\n");
        }
    }

    mysql_close(con);
    mysql_library_end();
    return 0;
}
