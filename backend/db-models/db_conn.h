//
// Created by artem on 01.05.26.
//

#ifndef REMINDER_DB_CONN_H
#define REMINDER_DB_CONN_H

#include <core-types.h>


/** @brief периоды задач в миллисекундах и тип сервера БД */
struct SProgCfg
{
    /** @brief из главного окна в главном потоке ткнуть палкой все ресурсы и дерево, что бы изучили работу своих потоков */
    uint32_t  pendingResTimeout_ms{5000};
    /** @brief период просмотра текущего набора событий на предмет активации оных */
    uint32_t  eventsTrackingPeriod_ms{1000};
    /** @brief период обновления публичного дерева (как само дерево, так и его ресурсы), на случай если админ его изменил (у админа эта задача не запускается) */
    uint32_t  publicNodesSyncPeriod_ms{10000};
    /** @brief Используемый в программе тип сервера БД */
    E_DB_TYPE   db_type{E_DB_TYPE::EDT_MYSQL};
};

/** @brief database connection parameters */
static SDBConnection sdb_connection
{
    .host = "192.168.1.101",
    .dbname = "reminder",
    .user = "artem",
    .pass = "masterkey",
    .ssl_config = SSslConf
    {
        .is_enabled = false
    },
    /*.ssl_config = SSslConf{.is_enabled = true, .ca_file = "..."}*/
    .is_data_encrypt_enabled = false
};



#endif //REMINDER_DB_CONN_H
