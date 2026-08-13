//
// Created by artem on 02.11.23.
//

#ifndef NYM_PROJECT_CUSERMODELDB_H
#define NYM_PROJECT_CUSERMODELDB_H


#include "units/CUser.h"
#include "../include/ADatabaseModel.h"


class CUserModelDB : public ADatabaseModel
{
public:
    CUserModelDB() = delete;
    CUserModelDB(const CUserModelDB &) = delete;
    explicit CUserModelDB(CAbstractConnection * connection);
    ~CUserModelDB() override;

    CUser login(const char * login, const char * pass);
    CUser   get_user_by_id(const Tsize &id);
    bool    add_user(CUser *user);
    bool    update_user(CUser *user);
    bool    remove_user(const Tsize &id);

    Tsize get_users(bool is_admin, std::vector<CUser *> *users);
    void    defineAdminId();

protected:


private:

    bool    get_user_from_row(CUser *user, std::vector<CDBValue*> *row);
    void    set_user_to_row(CUser *user, std::vector<CDBValue*> *row);
    bool    prepare_connection();
};


#endif //NYM_PROJECT_CUSERMODELDB_H
