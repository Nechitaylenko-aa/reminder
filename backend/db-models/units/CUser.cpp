//
// Created by artem on 31.05.22.
//

#include <cstring>
#include "CUser.h"


CUser::CUser(const CUser &src) {
    *this = src;
}

CUser::~CUser() {
    delete []  m_login;
    delete []  m_passwd_hash;
}

void CUser::set_string_data(char **variable, const char *value) {
    if (!value)
    {
        return;
    }

    delete [] *variable;
    Tsize len = strlen(value),
                n_len = len+1;

    *variable = new char[n_len];
    memcpy(*variable, value, len);
    char* var = *variable;
    var[len] = 0;
}

CUser &CUser::operator=(const CUser &rhs) {
    if (this == &rhs)
    {
        return *this;
    }

    m_id           = rhs.m_id;
    m_is_admin      = rhs.m_is_admin;

    if (rhs.m_passwd_hash)
    {
        set_pass_hash(rhs.get_pass_hash().c_str());
    }

    set_string_data(&m_login, rhs.m_login);


    return *this;
}

void CUser::set_pass_hash(const char *hash) {
    if (strlen(hash) < 32)
    {
        fprintf(stderr, "wtf: %s\n", hash);
    }

    delete [] this->m_passwd_hash;

    m_passwd_hash = new char [pass_hash_len + 1];
    memcpy(m_passwd_hash, hash, pass_hash_len);
    m_passwd_hash[pass_hash_len] = 0;
}


void CUser::set_login(const char *login) {
    set_string_data(&this->m_login, login);
}



bool CUser::operator==(const CUser &rhs) const
{
    if (
            m_id == rhs.m_id &&
            m_is_admin == rhs.m_is_admin &&
            strcmp(m_login, rhs.m_login) == 0


            )
        return true;

    return false;
}

Tstring CUser::get_pass_hash() const
{
    return m_passwd_hash == nullptr ? "" : m_passwd_hash;
}


void CUser::set_id(const Tsize &id_contact)
{
    m_id = id_contact;
    if (id_contact == 1)
    {
        m_is_admin = true;
    }
    else
    {
        m_is_admin = false;
    }

}

Tsize CUser::get_id() const
{
    return m_id;
}

void CUser::set_is_admin(bool is_admin)
{
    m_is_admin = is_admin;
}

bool CUser::is_admin() const
{
    return m_is_admin;
}

Tstring CUser::get_login() const
{
    return m_login == nullptr ? "not logged in" : m_login;
}
