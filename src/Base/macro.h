/*
 * macro.h
 *
 *  Created on: Nov 21, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base_macro_H_
#define OpenKAI_src_Base_macro_H_

#ifdef USE_GLOG
#define LOG_(x)         \
    {                   \
        LOG(INFO) << x; \
    }
#define LOG_I(x)                                   \
    if (m_bLog)                                    \
    {                                              \
        LOG(INFO) << this->getName() << ": " << x; \
    }
#define LOG_E(x)                                    \
    {                                               \
        LOG(ERROR) << this->getName() << ": " << x; \
    }
#define LOG_F(x)                                    \
    {                                               \
        LOG(FATAL) << this->getName() << ": " << x; \
    }
#else
#define LOG_(x)                    \
    {                              \
        string s = x;              \
        printf("%s\n", s.c_str()); \
    }
#define LOG_I(x)                               \
    if (m_bLog)                                \
    {                                          \
        string s = this->getName() + ": " + x; \
        printf("%s\n", s.c_str());             \
    }
#define LOG_E(x)                               \
    {                                          \
        string s = this->getName() + ": " + x; \
        printf("%s\n", s.c_str());             \
    }
#define LOG_F(x)                               \
    {                                          \
        string s = this->getName() + ": " + x; \
        printf("%s\n", s.c_str());             \
    }
#endif

#define IF_(x)  \
    if (x)      \
    {           \
        return; \
    }
#define IF_F(x)       \
    if (x)            \
    {                 \
        return false; \
    }
#define IF_N(x)         \
    if (x)              \
    {                   \
        return nullptr; \
    }
#define IF__(x, y) \
    if (x)         \
    {              \
        return y;  \
    }
#define IF_CONT(x) \
    if (x)         \
    {              \
        continue;  \
    }

#define NULL_(x)      \
    if (x == nullptr) \
    {                 \
        return;       \
    }
#define NULL_F(x)     \
    if (x == nullptr) \
    {                 \
        return false; \
    }
#define NULL_N(x)       \
    if (x == nullptr)   \
    {                   \
        return nullptr; \
    }
#define NULL__(x, y)  \
    if (x == nullptr) \
    {                 \
        return y;     \
    }

#define CHECK_(x)       \
    {                   \
        int r = x;      \
        if (r != OK_OK) \
        {               \
            return r;   \
        }               \
    }
#define CHECK_d_(x, y)  \
    {                   \
        int r = x;      \
        if (r != OK_OK) \
        {               \
            y;          \
            return r;   \
        }               \
    }
#define CHECK_d_l_(x, y, z) \
    {                       \
        int r = x;          \
        if (r != OK_OK)     \
        {                   \
            y;              \
            LOG_E(z);       \
            return r;       \
        }                   \
    }

#define DEL_ARRAY(x) \
    if (x)           \
    {                \
        delete[] x;  \
        x = NULL;    \
    }
#define DEL(x)       \
    if (x)           \
    {                \
        delete x;    \
        x = nullptr; \
    }
#define SWAP(x, y, t) \
    {                 \
        t = x;        \
        x = y;        \
        y = t;        \
    }

#define JO(o, n, v)                       \
    {                                     \
        o.insert(make_pair(n, value(v))); \
    }

#endif
