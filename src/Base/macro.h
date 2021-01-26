/*
 * macro.h
 *
 *  Created on: Nov 21, 2016
 *      Author: yankai
 */

#ifndef OpenKAI_src_Base_macro_H_
#define OpenKAI_src_Base_macro_H_

#define LOG_I(x) if(m_bLog){LOG(INFO)<<*this->getName()<<": "<<x;}
#define LOG_E(x) {LOG(ERROR)<<*this->getName()<<": "<<x;}
#define LOG_F(x) {LOG(FATAL)<<*this->getName()<<": "<<x;}

#define F_FATAL_F(x) if(x==false){LOG_F(#x);return false;}
#define F_ERROR_F(x) if(x==false){LOG_E(#x);return false;}
#define F_INFO(x) if(x==false){LOG_I(#x);}

#define F_(x) if(x==false){return;}
#define IF_T(x) if(x){return true;}
#define IF_F(x) if(x){return false;}
#define IF_N(x) if(x){return NULL;}
#define IF_(x) if(x){return;}
#define IF__(x,y) if(x){return y;}
#define IF_CONT(x) if(x){continue;}

#define F_l(x,y) if(x==false){LOG_E(y);return;}
#define IF_Fl(x,y) if(x){LOG_E(y);return false;}
#define IF_Nl(x,y) if(x){LOG_E(y);return NULL;}
#define IF_l(x,y) if(x){LOG_E(y);return;}
#define IF_NEGl(x,y) if(x){LOG_E(y);return -1;}

#define NULL_(x) if(x==NULL){return;}
#define NULL__(x,y) if(x==NULL){return y;}
#define NULL_F(x) if(x==NULL){return false;}
#define NULL_N(x) if(x==NULL){return NULL;}
#define NULL_T(x) if(x==NULL){return true;}

#define NULLl(x,y) if(x==NULL){LOG_E(y);}
#define NULL_l(x,y) if(x==NULL){LOG_E(y);return;}
#define NULL_Fl(x,y) if(x==NULL){LOG_E(y);return false;}
#define NULL_Nl(x,y) if(x==NULL){LOG_E(y);return NULL;}

#define DEL(x) if(x){delete x;x=NULL;}
#define DEL_ARRAY(x) if(x){delete[] x;x=NULL;}
#define SWAP(x,y,t) {t=x;x=y;y=t;}

#define JO(o,n,v) {o.insert(make_pair(n,value(v)));}

#endif
