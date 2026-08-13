//
// Created by artem on 31.05.22.
//

#ifndef DB_MANAGER_CUSER_H
#define DB_MANAGER_CUSER_H

//#include <core-types.h>
#include "../../sources/include/base-types.h"
#define pass_hash_len 32



class CUser {

public:
    CUser   () = default;
    CUser   (const CUser &src);
    ~CUser  ();


    bool    operator==(const CUser& rhs) const;
    CUser&  operator=(const CUser &rhs);
    void    set_pass_hash(const char *hash);    //!< using with GUI-login
    Tstring get_pass_hash() const;
    void    set_id(const Tsize &id_contact);
    Tsize   get_id() const;
    void    set_is_admin(bool is_admin);
    bool   is_admin() const;
    void    set_login(const char* login);
    Tstring get_login() const;


private:
    // service data `user`
    Tsize   m_id         {0};
    uint8_t m_is_admin    {0};

    char*   m_passwd_hash{nullptr};
    char*   m_login      {nullptr};


    static void    set_string_data(char **variable, const char* value);

};


#endif //DB_MANAGER_CUSER_H
