//
// Created by artem on 02.11.23.
//

#include "CUserModelDB.h"
#include <cstring>

uint32_t id_admin = 0;

CUserModelDB::CUserModelDB(CAbstractConnection *connection) : ADatabaseModel("users", connection)
{
    m_connection = connection;
}

CUserModelDB::~CUserModelDB()
= default;

bool CUserModelDB::prepare_connection()
{
    if (!m_connection->is_opened())
    {
        if (!m_connection->open())
        {
            return false;
        }
    }

    return true;
}

bool CUserModelDB::get_user_from_row(CUser *user, std::vector<CDBValue *> *row)
{

    user->set_id(row->at(0)->integer());
    user->set_login(row->at(1)->string().c_str());
    user->set_is_admin(row->at(2)->integer());
    user->set_pass_hash(row->at(3)->value().value.data_s);

    return true;
}

void CUserModelDB::set_user_to_row(CUser *user, std::vector<CDBValue *> *row)
{
    row->emplace_back(new CDBValue("login_s", user->get_login()));
    row->emplace_back(new CDBValue("is_admin_i", user->is_admin()));
    row->emplace_back(new CDBValue("passwd_s", CFunctions::md5_hash((Tstring)user->get_pass_hash())));
}

CUser CUserModelDB::login(const char *login, const char *pass)
{
    if (!prepare_connection() || !login || !pass)
    {
        return {};
    }

    if (id_admin == 0)
    {
        defineAdminId();
    }

    clear_all();

    SDBCondition cond, cond1;
    cond.name = "login_s";
    cond.value_s = login;
    cond.isString = true;

    cond1.name = "passwd_s";
    cond1.value_s = CFunctions::md5_hash((Tstring)pass);


    Tsize sz = strlen(pass);
    memset(&pass, 0, sz);


    cond1.isString = true;
    cond1.logic = ELogicOpers::ELO_AND;

    *(this->request->conditions) << cond << cond1;

    bool res = this->read(true);

    if (res)
    {
        auto row = answer->at(0);

        CUser user;
        get_user_from_row(&user, row);

        return user;
    }

    return {};
}

CUser CUserModelDB::get_user_by_id(const Tsize &id)
{
    if (!prepare_connection())
    {
        return {};
    }

    clear_all();

    SDBCondition cond;
    cond.name = "id";
    cond.value_s = std::to_string(id);

    this->request->conditions->push_back(cond);

    bool res = read(true);

    if (res)
    {
        auto row = answer->at(0);
        CUser user;

        get_user_from_row(&user, row);
        return user;
    }

    return {};
}

bool CUserModelDB::add_user(CUser *user)
{
    if (!prepare_connection())
    {
        return false;
    }

    clear_all();

    set_user_to_row(user, this->request->values);

    bool res = add(false);

    if (res)
    {
        user->set_id(get_last_id());
    }

    return res;
}

bool CUserModelDB::update_user(CUser *user)
{
    if (!prepare_connection())
    {
        return false;
    }

    clear_all();

    SDBCondition cond("id", std::to_string(user->get_id()));
    this->request->conditions->push_back(cond);

    set_user_to_row(user, this->request->values);

    bool res = this->write();

    return res;
}

bool CUserModelDB::remove_user(const Tsize &id)
{
    if (!prepare_connection())
    {
        return false;
    }

    clear_all();

    SDBCondition cond("id", std::to_string(id));
    this->request->conditions->push_back(cond);

    return remove();
}

Tsize CUserModelDB::get_users(bool is_admin, std::vector<CUser *> *users)
{
    if (!prepare_connection())
    {
        return 0;
    }

    clear_all();

    SDBCondition cond("is_admin_i", std::to_string(is_admin));
    this->request->conditions->push_back(cond);

    auto res = read(true);

    if (res)
    {
        CUser user;

        for (auto &row : *answer)
        {
            get_user_from_row(&user, row);
            users->emplace_back(new CUser(user));
        }
    }

    return res;
}

void CUserModelDB::defineAdminId()
{
    if (!prepare_connection() || id_admin > 0)
    {
        return;
    }

    request->conditions->push_back(SDBCondition("is_admin_i", "1"));
    bool res = read(true);
    if (!res || answer->empty())
    {
        return;
    }

    CUser admin;
    get_user_from_row(&admin, answer->front());
    id_admin = admin.get_id();
}
