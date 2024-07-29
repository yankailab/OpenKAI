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
#define NULL__(x, y)  \
    if (x == nullptr) \
    {                 \
        return y;     \
    }
#define NULL_N(x)       \
    if (x == nullptr)   \
    {                   \
        return nullptr; \
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





#define IF_F(x)       \
    if (x)            \
    {                 \
        return false; \
    }
#define IF_N(x)      \
    if (x)           \
    {                \
        return NULL; \
    }
#define F_FATAL_F(x)  \
    if (x == false)   \
    {                 \
        LOG_F(#x);    \
        return false; \
    }
#define F_ERROR_F(x)  \
    if (x == false)   \
    {                 \
        LOG_E(#x);    \
        return false; \
    }
#define F_INFO(x)   \
    if (x == false) \
    {               \
        LOG_I(#x);  \
    }

#define IF_T(x)      \
    if (x)           \
    {                \
        return true; \
    }

#define IF_d_T(x, y) \
    if (x)           \
    {                \
        y;           \
        return true; \
    }
#define IF_d_F(x, y)  \
    if (x)            \
    {                 \
        y;            \
        return false; \
    }
#define IF_d_N(x, y) \
    if (x)           \
    {                \
        y;           \
        return NULL; \
    }
#define IF_d_(x, y) \
    if (x)          \
    {               \
        y;          \
        return;     \
    }
#define IF_d__(x, y, z) \
    if (x)              \
    {                   \
        y;              \
        return z;       \
    }

#define IF_Fl(x, y)   \
    if (x)            \
    {                 \
        LOG_E(y);     \
        return false; \
    }
#define IF_Nl(x, y)  \
    if (x)           \
    {                \
        LOG_E(y);    \
        return NULL; \
    }
#define IF_l(x, y) \
    if (x)         \
    {              \
        LOG_E(y);  \
        return;    \
    }

#define NULL_F(x)     \
    if (x == NULL)    \
    {                 \
        return false; \
    }
#define NULL_T(x)    \
    if (x == NULL)   \
    {                \
        return true; \
    }

#define NULL_d_(x, y) \
    if (x == NULL)    \
    {                 \
        y;            \
        return;       \
    }
#define NULL_d__(x, y, z) \
    if (x == NULL)        \
    {                     \
        y;                \
        return z;         \
    }
#define NULL_d_F(x, y) \
    if (x == NULL)     \
    {                  \
        y;             \
        return false;  \
    }
#define NULL_d_N(x, y) \
    if (x == NULL)     \
    {                  \
        y;             \
        return NULL;   \
    }
#define NULL_d_T(x, y) \
    if (x == NULL)     \
    {                  \
        y;             \
        return true;   \
    }

#define NULLl(x, y) \
    if (x == NULL)  \
    {               \
        LOG_E(y);   \
    }
#define NULL_l(x, y) \
    if (x == NULL)   \
    {                \
        LOG_E(y);    \
        return;      \
    }
#define NULL_Fl(x, y) \
    if (x == NULL)    \
    {                 \
        LOG_E(y);     \
        return false; \
    }
#define NULL_Nl(x, y) \
    if (x == NULL)    \
    {                 \
        LOG_E(y);     \
        return NULL;  \
    }

#define DEL_ARRAY(x) \
    if (x)           \
    {                \
        delete[] x;  \
        x = NULL;    \
    }

#define JO(o, n, v)                       \
    {                                     \
        o.insert(make_pair(n, value(v))); \
    }

#endif
