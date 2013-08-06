/*****************************************************************************
 * Copyright(C), 2013 ReadConfig
 *
 * Description:That's simple class
 * Written by yexf
 * ***************************************************************************/
#ifndef __BASE_TREADCONF_H
#define __BASE_TREADCONF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>

#include <map>

#define __TREADCONF_BUFFER_LENGTH_MAX 1024

static inline void TrimRight(char *src) {
    char *end = src + strlen(src) - 1;
    for (; *end == ' ' && end >= src; --end) *end = 0;
};

namespace ReadConfContainer {
    typedef struct {
        char name[32 + 1];
        char item[32 + 1];
    } CONF_KEY_INFO;

    typedef struct {
        char text[__TREADCONF_BUFFER_LENGTH_MAX + 1];
    } CONF_VAL_INFO;

    struct conf_ltcom {
        bool operator() (const CONF_KEY_INFO &x1, const CONF_KEY_INFO &x2) const {
            return memcmp(&x1,&x2,sizeof(CONF_KEY_INFO)) < 0;
        }
    };

    typedef std::map<CONF_KEY_INFO, CONF_VAL_INFO, conf_ltcom> ConfigEntry;
    typedef std::map<CONF_KEY_INFO, CONF_VAL_INFO, conf_ltcom>::iterator ConfigEntryIt;
    typedef std::map<CONF_KEY_INFO, CONF_VAL_INFO, conf_ltcom>::const_iterator ConstConfigEntryIt;
}

class TReadConf {
    public:
        typedef ReadConfContainer::CONF_KEY_INFO keyType;
        typedef ReadConfContainer::CONF_VAL_INFO valType;
        typedef ReadConfContainer::ConfigEntry containerType;
        typedef ReadConfContainer::ConfigEntryIt ConfigEntryIt;
        typedef ReadConfContainer::ConstConfigEntryIt ConConfigEntryIt;

    public:

        void Clear() {
            m_mEntry.clear();
        }

        /****************************************************************************
         * Load Data
         * const char *pathname :
         * *************************************************************************/
        int Init(const char *pathname) {
            FILE *fp = NULL;
            size_t itemLength = 0;
            size_t currentpos = 0;
            size_t lastpos    = 0;
            ssize_t iPos = 0;
            char *pos = NULL;
            char *sip = NULL;
            char buffer[__TREADCONF_BUFFER_LENGTH_MAX] = {0};
            keyType key;
            valType val;

            fp = fopen(pathname, "r");
            if (fp == NULL) {
                /* printf("Cannot open file[%s]\n", pathname); */
                return -1;
            }
            memset(&key, 0x00, sizeof(keyType));
            while ((fgets(buffer, __TREADCONF_BUFFER_LENGTH_MAX, fp)) != 0) {
                currentpos = ftell(fp);
                buffer[currentpos - lastpos - 1] = 0;
                if ((currentpos - lastpos) < __TREADCONF_BUFFER_LENGTH_MAX) {
                    lastpos = currentpos;
                } else {
                    /* 将超过__TREADCONF_BUFFER_LENGTH_MAX的内容默认丢弃 */
                    char TempMorebuffer[__TREADCONF_BUFFER_LENGTH_MAX] = {0};

                    while (fgets(TempMorebuffer, __TREADCONF_BUFFER_LENGTH_MAX, fp) != 0) {
                        currentpos = ftell(fp);
                        if ((currentpos - lastpos) >= __TREADCONF_BUFFER_LENGTH_MAX) {
                            lastpos = currentpos;
                            continue;
                        } else {
                            lastpos = currentpos;
                            break;
                        }
                    }
                }

                iPos = 0;
                pos = buffer;
                while (*pos == ' ') {
                    ++iPos;
                    ++pos;
                }

                if (buffer[iPos] == '#') {
                    memset(buffer, 0x00, __TREADCONF_BUFFER_LENGTH_MAX);
                    continue;
                }

                memset(&key.item, 0x00, sizeof(keyType::item));
                memset(&val, 0x00, sizeof(valType));
                if (buffer[iPos] == '[') {
                    ++iPos;
                    sip = strchr(buffer + iPos, ']');
                    if (sip == NULL) return -1;

                    memset(&key.name, 0x00, sizeof(keyType::name));
                    itemLength = sip - buffer - iPos;
                    if (itemLength >= 32)
                        memcpy(key.name, buffer + iPos, 32);
                    else
                        memcpy(key.name, buffer + iPos, itemLength);

                    TrimRight(key.name);
                    iPos += sip - buffer;
                    continue;
                }

                sip = strchr(buffer + iPos, '=');
                if (sip == NULL) {
                    memset(buffer, 0x00, __TREADCONF_BUFFER_LENGTH_MAX);
                    continue;
                }

                itemLength = sip - buffer - iPos;
                if (itemLength >= 32)
                    memcpy(key.item, buffer + iPos, 32);
                else
                    memcpy(key.item, buffer + iPos, itemLength);

                TrimRight(key.item);

                pos = sip + 2;
                iPos = sip - buffer + 2;
                while (*pos == ' ') {
                    ++iPos;
                    ++pos;
                }

                /* ign # */
                sip = strchr(buffer + iPos, '#');
                if (sip != NULL)
                    *sip = '\0';

                strcpy(val.text, pos);
                TrimRight(val.text);

                ConfigEntryIt it = m_mEntry.find(key);
                if (it == m_mEntry.end()) {
                    m_mEntry.insert(std::pair<keyType, valType>(key, val));
                } else {
                    strcpy(it->second.text, val.text);
                }
            }
            fclose(fp);

            return 0;
        }

        void Display() const
        {
            for (ConConfigEntryIt it = m_mEntry.begin(); it != m_mEntry.end(); ++it) {
                printf("Key:[%s][%s], Value:[%s]\n", it->first.name, it->first.item, it->second.text);
            }
        }

        int GetItem(const char *name, const char *item, char *value, const size_t &valen) const {
            keyType key;

            memset(&key, 0x00, sizeof(keyType));
            memset(value, 0x00, valen);

            strncpy(key.name, name, 32);
            strncpy(key.item, item, 32);
            ConConfigEntryIt it = m_mEntry.find(key);
            if (it == m_mEntry.end()) {
                return -1;
            }

            strncpy(value, it->second.text, valen);
            return 0;
        }

    private:
        containerType m_mEntry;

};

#endif /* __BASE_TREADCONF_H */
