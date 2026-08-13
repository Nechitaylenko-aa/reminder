//
// Created by artem on 28.04.26.
//

#ifndef REMINDER_BASE_TYPES_H
#define REMINDER_BASE_TYPES_H

#include <cstdint>
#include <string>
#include <map>
#include <db-types.h>

enum  ResourceType : uint8_t
{
    TEXT = 0,
    DATES,
    RT_COUNT
};

enum EventPriority : uint8_t {
    EP_NORMAL,    // Обычное — синее 🟢 (или спокойный синий)
    EP_IMPORTANT, // Важное — оранжевое 🟠
    EP_CRITICAL,   // Критичное — красное 🔴
    EVEPRIOR_COUNT
};

static Tstring priorityName[EVEPRIOR_COUNT]
{
    "обычное",
    "важное",
    "критическое"
};

enum EventPeriod : uint8_t
{
    EP_NONE,
    EP_MINUTE,
    EP_HOURLY,
    EP_DAILY,
    EP_WEEKLY,
    EP_MONTH,
    EP_YEAR,
    EP_COUNT
};

static Tstring periodText[EP_COUNT]
{
    "Разовое",
    "минута",
    "час",
    "сутки",
    "неделя",
    "месяц",
    "год"
};

enum EventType : uint8_t
{
    ET_DATE,
    ET_TIME,
    ET_DATE_TIME,
    EvT_COUNT
};

static Tstring  eventTypeNames[EvT_COUNT]
{
    "Дата",
    "Время",
    "Дата и время"
};

/**@brief триггер сработки если событие приходится на праздники/выходные: строго в день /перед / после / никогда */
enum EventTrigger : uint8_t
{
    ET_STRICTLY,
    ET_BEFORE,
    ET_AFTER,
    ET_NEVER,
    ET_COUNT
};

static Tstring  triggerName[ET_COUNT]
{
    "строго",
    "перед",
    "после",
    "никогда"
};

enum class TextEntryType : uint8_t
{
    PLAIN_TEXT,  // Обычный текст
    URI_LINK,    // Ссылки (http, ftp, smb)
    FILE_SYSTEM, // Локальные пути
    SECRET       // Пароли (маскированный ввод)
};

struct TextEntry {
    uint32_t id;
    uint32_t id_parent;
    TextEntryType type;   // Тип записи
    Tstring desc;     // Описание/Заголовок
    Tstring data;     // Контент (текст, url или путь)
    bool is_removed{false};
    bool showSecret{false}; // чисто локально показывать/скрывать секретную инфу

    bool operator==(const TextEntry& other) const {
        return id == other.id &&
                id_parent == other.id_parent &&
                desc == other.desc &&
                data == other.data &&
                type == other.type;
    }

    bool operator!=(const TextEntry& other) const {
        return !(*this == other);
    }
};

struct EventEntry
{
    uint32_t    id{0};
    uint32_t    id_parent{0};
    EventPeriod period{EventPeriod::EP_MONTH};
    uint16_t    period_count{1};
    time_t      event{0};
    time_t      was_shown{0};
    EventType   type{EventType::ET_DATE};
    EventTrigger trigger{ET_STRICTLY};
    Tstring     description;
    EventPriority priority = EP_NORMAL;
    bool        isEnabled{true};
    bool        is_removed{false};

    bool operator==(const EventEntry& other) const {
        return id == other.id &&
               id_parent == other.id_parent &&
               period == other.period &&
               period_count == other.period_count &&
               event == other.event &&
               was_shown == other.was_shown &&
               type == other.type &&
               trigger == other.trigger &&
               description == other.description &&
               priority == other.priority &&
               isEnabled == other.isEnabled &&
               is_removed == other.is_removed;
    }

    bool operator!=(const EventEntry& other) const {
        return !(*this == other);
    }
};

struct SNode {
    uint32_t id{};
    uint32_t parent_id{};
    uint32_t user_id{};
    Tstring title{};
    ResourceType res_type{};

    bool is_public{};         // Находится ли в ветке "Общее"
    bool is_editable{};       // Можно ли менять саму ноду (имя, перемещение)
    bool is_container{};      // В этой папке можно хранить данные? (для рутовых и 2-го уровня false
    bool is_admin{};
    bool is_removed{false};

    [[nodiscard]] bool can_modify_data() const
    {
        return !is_public || is_admin;
    }
    SNode() = default;
    SNode& operator=(const SNode&) = default;
    SNode(const SNode&) = default;
    bool operator==(const SNode& other) const {
        return id == other.id &&
               parent_id == other.parent_id &&
               user_id == other.user_id &&
               title == other.title &&
               res_type == other.res_type &&
               is_public == other.is_public &&
               is_editable == other.is_editable &&
               is_container == other.is_container &&
               is_admin == other.is_admin &&
               is_removed == other.is_removed;
    }

    bool operator!=(const SNode& other) const {
        return !(*this == other);
    }
};

#endif //REMINDER_BASE_TYPES_H
