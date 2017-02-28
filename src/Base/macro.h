/*
 * macro.h
 *
 *  Created on: Nov 21, 2016
 *      Author: root
 */

#ifndef OPENKAI_SRC_BASE_MACRO_H_
#define OPENKAI_SRC_BASE_MACRO_H_

#define LOG_I(x) if(m_bLog){LOG(INFO)<<*this->getName()<<": "<<x;}
#define LOG_E(x) if(m_bLog){LOG(ERROR)<<*this->getName()<<": "<<x;}
#define LOG_F(x) if(m_bLog){LOG(FATAL)<<*this->getName()<<": "<<x;}

#define F_FATAL_F(x) if(x==false){LOG_F(#x);return false;}
#define F_ERROR_F(x) if(x==false){LOG_E(#x);return false;}
#define F_INFO(x) if(x==false){LOG_I(#x);}
#define F_(x) if(x==false){return;}

#define IF_T(x) if(x){return true;}
#define IF_F(x) if(x){return false;}
#define IF_N(x) if(x){return NULL;}
#define IF_(x) if(x){return;}
#define IF_CONT(x) if(x){continue;}

#define NULL_(x) if(x==NULL){return;}
#define NULL_F(x) if(x==NULL){return false;}
#define NULL_N(x) if(x==NULL){return NULL;}
#define NULL_T(x) if(x==NULL){return true;}

#define DEL(x) if(x){delete x;}
#define SWAP(x,y,t) {t=x;x=y;y=t;}

#endif
