// Copyright (c) 2023-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
/*
Based on [Ekker 1994] Appendix. B, but modified a bit.
(1) All values should be provided in MKS units. (angles should be in rad.)
(2) Transonic region (M=0.95 to 1.2) is also calculated by same method as either sub/supersonic region to simplify, although it is not so accuarate.
(3) Extended to tapered planform and canard-controlled missiles based on [Pitts 53](NACA TR 1307.)
(4) Induced drag on tail is calculated with the effect of downwash, which [Ekker 94] seems to be ignore by dε/dα≒1.
*/
#include "EkkerMDTV2.h"
#include <string>
#include <map>
#include <boost/math/special_functions/ellint_2.hpp>
#include "MathUtility.h"
#include "Units.h"

namespace py=pybind11;

ASRC_NAMESPACE_BEGIN(asrc)
ASRC_NAMESPACE_BEGIN(core)

using namespace util;

namespace EkkerMDTV2{
    double areapogv(double d,double ln){
        double R=d/4+ln*ln/d;
        double a=R-d/2;
        return ln*sqrt(R*R-ln*ln)+R*R*asin(ln/R)-2*a*ln;
    }
    double bdycla(double d,double l,double ln,double Mach,double alpha){
        double Sref=M_PI*d*d/4;
        double cdc=sscfdccc(alpha,Mach);
        double Ap=d*(l-ln)+areapogv(d,ln);
        double n;
        if(Mach<=1){
            n=dragfctr(l/d);
        }else{
            n=1.0;
        }
        return k2mk1(l/d)*2+n*Ap*cdc*alpha/Sref;
    }
    double bigkbwa(double M,double A,double r,double s,double taper){
        double b=sqrt(abs(M*M-1));
        double mb=b*A/4*(1+taper)/(1-taper);//m=cotΛ
        if(mb<=taper){
            //slender body theory
            //Eq. (21) in NACA TR 1307
            return bigkbwsb(r,s);
        }else{
            if(mb<1){
                //Eq. (28) in NACA TR 1307
                return bkbwsba(M,A,r,s,taper);
            }else{
                //Eq. (29) in NACA TR 1307
                return bkbwspa(M,A,r,s,taper);
            }
        }
    }
    double bigkbwna(double M,double A,double r,double s,double taper){
        double b=sqrt(abs(M*M-1));
        double mb=b*A/4*(1+taper)/(1-taper);//m=cotΛ
        if(mb<=taper){
            //slender body theory
            //Eq. (21) in NACA TR 1307
            return bigkbwsb(r,s);
        }else{
            if(mb<1){
                //Eq. (31) in NACA TR 1307
                return bkbwsbna(M,A,r,s,taper);
            }else{
                //Eq. (30) in NACA TR 1307
                return bkbwspna(M,A,r,s,taper);
            }
        }
    }
    double bigkbwsb(double r,double s){
        //NACA TR 1307 Eq. (21)
        double t=r/s;
        double Kbw;
        if(t>=1){
            Kbw=2.;
        }else{
            double t1=1/t;
            Kbw=(1+pow(t,4))*(0.5*atan(0.5*(t1-t))+M_PI/4)-t*t*((t1-t)+2*atan(t));
            Kbw=(pow(1-t*t,2)-2*Kbw/M_PI)/pow(1-t,2);
        }
        return Kbw;
    }
    double bigkwb(double r,double s){
        //NACA TR 1307 Eq. (14)
        double t=r/s;
        double Kbw;
        if(t>1){
            Kbw=2.;
        }else{
            double t1=1/t;
            Kbw=(1+pow(t,4))*(0.5*atan(0.5*(t1-t))+M_PI/4)-t*t*((t1-t)+2*atan(t));
            Kbw=2*Kbw/M_PI/pow(1-t,2);
        }
        return Kbw;
    }
    double bkbwsba(double M,double A,double r,double s,double taper){
        //NACA TR 1307 Eq. (26)
        double b=sqrt(abs(M*M-1));
        double t=r/s;
        double m=A/4*(1+taper)/(1-taper);
        double mb=m*b;
        double cr=4*(s-r)/A/(1+taper);
        double dcr=2*r/cr;
        double bdcr=b*dcr;
        double t1=(1+mb)*dcr/m;
        double Kbw=pow(1+t1,1.5)+pow(1+t1,0.5)-2-t1*t1*atanh(sqrt(1/(1+t1)));
        double k=sqrt(abs(1-mb*mb));
        if(std::isnan(k)){
            // 0<=mb<1のときに呼ばれる関数なのでk>0になるはずだが、
            // 浮動小数点数の誤差によりごく稀にkがNaNになる。
            // その場合はmを0とみなしてE=π/2を得る。
            k=0;
        }
        double E=boost::math::ellint_2(k);
        Kbw=16*E*m*Kbw/(A*pow(M_PI*(mb+1),2));
        return Kbw;
    }
    double bkbwsbna(double M,double A,double r,double s,double taper){
        //NACA TR 1307 Eq. (31)
        double b=sqrt(abs(M*M-1));
        double t=r/s;
        double m=A/4*(1+taper)/(1-taper);
        double mb=m*b;
        double cr=4*(s-r)/A/(1+taper);
        double dcr=2*r/cr;
        double bdcr=b*dcr;
        double mcrd=m/dcr;
        double k=sqrt(abs(1-mb*mb));
        if(std::isnan(k)){
            // 0<=mb<1のときに呼ばれる関数なのでk>0になるはずだが、
            // 浮動小数点数の誤差によりごく稀にkがNaNになる。
            // その場合はmを0とみなしてE=π/2を得る。
            k=0;
        }
        double E=boost::math::ellint_2(k);
        double Kbw;
        if(bdcr<1){
            Kbw=(1+mcrd)*sqrt((1-bdcr)*(1+dcr/m))-mcrd;
            Kbw=Kbw-(mb+1)/mcrd*atanh(sqrt((1-bdcr)/(1+dcr/m)));
            if(b<1e-3){
                Kbw=Kbw+mcrd*(mb+1)*(sqrt((1+dcr/m)/(1-bdcr))*atanx_x(sqrt((mb+bdcr)/(1-bdcr)))-atanx_x(sqrt(mb)));
            }else{
                Kbw=Kbw+mcrd*(mb+1)/sqrt(mb)*(atan2(sqrt(mb+bdcr),sqrt(1-bdcr))-atan(sqrt(mb)));
            }
            Kbw=16*E*dcr*Kbw/(M_PI+M_PI*A*(mb+1));
        }else{
            Kbw=16*E/(M_PI*M_PI*A*b)*(sqrt(mb)*atan(1/sqrt(mb))-mb/(mb+1));
        }
        return Kbw;
    }
    double bkbwspa(double M,double A,double r,double s,double taper){
        //NACA TR 1307 Eq. (24)
        double b=sqrt(abs(M*M-1));
        double t=r/s;
        double m=A/4*(1+taper)/(1-taper);
        double mb=m*b;
        double cr=4*(s-r)/A/(1+taper);
        double dcr=2*r/cr;
        double bdcr=b*dcr;
        double t1=(1+mb)*bdcr;
        double Kbw=mb/(1+mb)*pow(1/mb+1,2)*acos((1+t1)/(mb+t1));
        Kbw=Kbw+sqrt(mb*mb-1)/(1+mb)*(sqrt(1+2*bdcr)-1);
        Kbw=Kbw-sqrt(mb*mb-1)/mb*bdcr*bdcr*acosh(1+1/bdcr);
        Kbw=Kbw-mb/(1+mb)*acos(1/mb);
        Kbw=4*m/(M_PI*A*sqrt(mb*mb-1))*Kbw;
        return Kbw;
    }
    double bkbwspna(double M,double A,double r,double s,double taper){
        //NACA TR 1307 Eq. (30)
        double b=sqrt(abs(M*M-1));
        double t=r/s;
        double m=A/4*(1+taper)/(1-taper);
        double mb=m*b;
        double cr=4*(s-r)/A/(1+taper);
        double crd=cr/(2*r);
        double bdcr=b/crd;
        double Kbw;
        if(bdcr<1){
            Kbw=mb*pow(1/bdcr,2)*asin(bdcr);
            Kbw=Kbw-acosh(1/bdcr);
            if(sqrt(mb*mb-1)<1e-3){
                Kbw=Kbw-pow(m*crd,2)/sqrt(mb+1)*acosOne_OnePlusx2_x(sqrt(mb-1));
                double t1=sqrt((m*crd-mb)/(mb*mb+m*crd));
                Kbw=Kbw+pow(1+m*crd,2)/sqrt(mb+1)*t1*acosOne_OnePlusx2_x(t1*sqrt(mb-1));
            }else{
                Kbw=Kbw-pow(m*crd,2)*acos(1/mb)/sqrt(mb*mb-1);
                Kbw=Kbw+pow(1+m*crd,2)/sqrt(mb*mb-1)*acos((mb+1/bdcr)/(1+m*crd));
            }
            Kbw=4/(M_PI*A*b)*bdcr*bdcr*Kbw;
        }else{
            Kbw=4/(M_PI*A*b)*(M_PI_2*mb-mb*mb/sqrt(mb*mb-1)*acos(1/mb));
        }
        return Kbw;
    }
    double bsdrgcf(double Mach){
        static const Eigen::VectorXd p{{
            -3.330968357195938e4,
            2.103304015137906e5,
            -5.784664805761514e5,
            9.050874924571224e5,
            -8.811731632140930e5,
            5.466276717656434e5,
            -2.109994072780218e5,
            4.633522170518622e4,
            -4.431983251061754e3
        }};
        return polyval(p,Mach);
    }
    double bsdrgsp(double Mach){
        static const Eigen::VectorXd p{{
            -2.903303569625142e-5,
            8.917615745536690e-4,
            -1.171998069776006e-2,
            8.580630708865064e-2,
            -3.811230276820538e-1,
            1.044387398043697e0,
            -1.697460661376502e0,
            1.405711843047642e0,
            -2.362628974278631e-1
        }};
        if(Mach>5.58){
            // 近似多項式がMach～5.585に変曲点を持ち、これより大きいところで近似が成立しなくなる。
            // この関数はMach→∞で2/(γ*Mach^2)に下側から漸近するので、Machが大きいところでは
            // そのような性質を持つ別の近似式に切り替える。
            // 具体的には、Mach>5.58の領域では比熱比γを1.4としてf(M)=(2/1.4)/(M^2+A+C*exp(-λ*(x-5.58)))の形で近似することとし、
            // Mach=5.58において両近似式とその1階、2階微分の値が一致するようにA、C、λを定める。
            double A=2.63035490444532;
            double C=0.176893774514883;
            double lam=4.81581793472283;
            return (2/1.4)/(Mach*Mach+A+C*exp(-lam*(Mach-5.58)));
        }
        return -polyval(p,Mach);
    }
    double cdbbody(double d,double l,double ln,double Mach,double atr){
        double Sref=M_PI*d*d/4;
        double cdc=sscfdccc(atr,Mach);
        double Ap=d*(l-ln)+areapogv(d,ln);
        double n;
        if(Mach<=1){
            n=dragfctr(l/d);
        }else{
            n=1.0;
        }
        return atr*atr*(2*k2mk1(l/d)+n*cdc*Ap*atr/Sref);
    }
    double cdlcomp(double bw,double Sw,double taperw,double Mach){
        double beta=sqrt(Mach*Mach-1);
        double Aw=bw*bw/Sw;
        static const Eigen::VectorXd p{{
            -1.131676150597421e0,
            7.455524067331654e0,
            -1.959790137335054e1,
            2.617725155474472e1,
            -1.909733706084593e1,
            8.093378527305321e0,
            -1.217830853307086e0,
            5.365069933169143e-1
        }};
        return polyval(p,beta*Aw/4*(1+taperw));
    }
    double cdlwing(double bw,double Sw,double taperw,double Mach_,double atr){
        double Aw=bw*bw/Sw;
        double Mach=Mach_>0?Mach_:0.0001;
        if(atr==0){
            return 0.0;
        }
        if(Mach<=1){
            double kappaw=0.85;
            double lc2w=atan(2/Aw*(1-taperw)/(1+taperw));
            double CLaW=clawsub(Mach,Aw,taperw,lc2w,kappaw);
            return CLaW*atr*atr/1.1;
        }else{
            double betaw=sqrt(Mach*Mach-1);
            double m=Aw/4*(1+taperw)/(1-taperw);
            if(m*betaw>=1){
                return 4*atr*atr/betaw;
            }else{
                double corr=cdlcomp(bw,Sw,taperw,Mach);
                double CLaW=clawsup(Mach,Aw,taperw);
                return (3+taperw)/(1+taperw)*pow(CLaW*atr,2)*corr/M_PI/Aw;
            }
        }
    }
    double cdobody(double d,double l,double ln,double Mach,double alt,double nf,bool pwroff){
        double Sref=M_PI*d*d/4.;
        double Swet=surfogv1(d,ln)+(l-ln)*M_PI*d;
        double cdo,CDF;
        if(Mach<=0){
            cdo=0.0;
        }else if(Mach<=1){
            double Mach1=std::min(0.6,Mach);
            double cf=nf*cfturbfp(l,Mach1,alt);
            CDF=cf*Swet/Sref;
            double CDP=cf*(60/pow(l/d,3)+0.0025*l/d)*Swet/Sref;
            cdo=CDF+CDP;
        }else{
            double cf=nf*cfturbfp(l,Mach,alt);
            double CDW=wvdrgogv(d,ln,Mach);
            double CDF=cf*Swet/Sref;
            double CDP;
            if(Mach<1.2){
                double cf=nf*cfturbfp(l,0.6,alt);
                double CDPp6=cf*(60/pow(l/d,3)+0.0025*l/d)*Swet/Sref;
                CDP=CDPp6*(1.2-Mach)/0.2;
            }else{
                CDP=0;
            }
            cdo=CDF+CDW+CDP;
        }
        if(pwroff){
            double cdb;
            if(Mach<=0){
                cdb=0;
            }else if(Mach<=1){
                cdb=0.029/sqrt(CDF);
                if(Mach>0.6){
                    cdb=cdb+bsdrgcf(Mach);
                }
            }else{
                cdb=-bsdrgsp(Mach);
            }
            cdo=cdo+cdb;
        }
        return cdo;
    }
    double cdowbt(double d,double l,double ln,double bw,double Sw,double thickw,double taperw,double bt,double St,double thickt,double tapert,double Mach,double alt,double nf,bool pwroff){
        double Sref=M_PI*d*d/4.;
        if(Mach<=0.95 || Mach>=1.1){
            double cdob=cdobody(d,l,ln,Mach,alt,nf,pwroff);
            double cdow=cdowing(bw,Sw,thickw,taperw,Mach,alt,nf);
            double cdot=cdowing(bt,St,thickt,tapert,Mach,alt,nf);
            return cdob+2*cdow*Sw/Sref+2*cdot*St/Sref;
        }else{
            double cdop95=cdowbt(d,l,ln,bw,Sw,thickw,taperw,bt,St,thickt,tapert,0.95,alt,nf,pwroff);
            double cdo1p1=cdowbt(d,l,ln,bw,Sw,thickw,taperw,bt,St,thickt,tapert,1.1,alt,nf,pwroff);
            double dCdM=(cdo1p1-cdop95)/0.15;
            return std::min(cdo1p1,dCdM*(Mach-0.95)+cdop95);
        }
    }
    double cdowing(double bw,double Sw,double thick,double taper,double Mach_,double alt,double nf){
        double Mach=Mach_==1.0?0.999999:Mach_;
        double MAC=(2.0/3.0)*(2*Sw/bw/(1+taper))*(1+taper+taper*taper)/(1+taper);
        if(Mach==0){
            return 0.0;
        }else if(Mach<=1){
            double cf=nf*cfturbfp(MAC,Mach,alt);
            return 2*cf*(1+1.2*thick+60*pow(thick,4));
        }else{
            double cf=nf*cfturbfp(MAC,Mach,alt);
            double beta=sqrt(Mach*Mach-1.);
            double B=4.;
            double m=bw*bw/Sw/4*(1+taper)/(1-taper); //[Ekker 1994] seems to have mishandled degree and radian for SWP in this function.
            double cdw;
            if(m*beta>=1){
                cdw=B*thick*thick/beta;
            }else{
                cdw=B*m*thick*thick;
            }
            return 2*cf+cdw;
        }
    }
    double cdtrim(double d,double l,double ln,double bw,double Sw,double thickw,double taperw,double lw,double bt,double St,double thickt,double tapert,double Mach,double alt,double atr,double dtr,double nf,bool pwroff, bool isTailControlled){
        double Sref=M_PI*d*d/4.;
        double CDoB=cdobody(d,l,ln,Mach,alt,nf,pwroff);
        double CDoW=cdowing(bw,Sw,thickw,taperw,Mach,alt,nf);
        double CDoT=cdowing(bt,St,thickt,tapert,Mach,alt,nf);
        //Downwash
        double deda;
        if(Mach<1){
            double Aw=bw*bw/Sw;
            double lc2w=atan(2/Aw*(1-taperw)/(1+taperw));
            double lc4w=atan(3/Aw*(1-taperw)/(1+taperw));
            double kappaw=0.85;
            double CLaW=clawsub(Mach,Aw,taperw,lc2w,kappaw);
            double CLaW0=clawsub(0,Aw,taperw,lc2w,kappaw);
            double lH=l-St/bt*(1+tapert*tapert/(1+tapert))/(1+tapert)-(lw+Sw/bw*(1-taperw*taperw/(1+taperw))/(1+taperw));
            deda=dedasub(Aw,bw,lc4w,taperw,lH,0)*CLaW/CLaW0;
        }else{
            double crw=2*Sw/bw/(1+taperw);
            double crt=2*St/bt/(1+tapert);
            double lt=l-crt;
            double x0=(lt-(lw+crw))/crw;
            double sw=0.5*(d+bw);
            double Kwb=bigkwb(d/2,sw);
            double SWPwing=atan(2*crw/bw*(1-taperw));
            deda=Kwb*dedasup(Mach,SWPwing,x0);
        }
        if(isTailControlled){
            double CDoWB=CDoB+2*CDoW*Sw/Sref+CDoT*St/Sref;
            double CDLW=cdlwing(bw,Sw,taperw,Mach,atr);
            double CDBa=cdbbody(d,l,ln,Mach,abs(atr));
            double CDiWB=CDLW*Sw/Sref+CDBa;
            double Mt=0.95*Mach;
            double CDLT=cdlwing(bt,St,tapert,Mt,atr*(1-deda)+dtr);//ASRC: Downwash is taken into account.
            double CDT=CDoT+CDLT;
            double CLd=cldwbt_ndw(d,bt,St,tapert,Mach,isTailControlled)*Sref/St;
            double CLT=CLd*dtr;
            double nt=pow(Mt/Mach,2);
            double CDTR=(CDT*cos(dtr)+CLT*sin(dtr))*St*nt/Sref;
            return CDoWB+CDiWB+CDTR;
        }else{
            double CDoTB=CDoB+CDoW*Sw/Sref+2*CDoT*St/Sref;
            double Mt=0.95*Mach;
            double CDLT=cdlwing(bt,St,tapert,Mt,atr*(1-deda)-deda*dtr);//ASRC: Downwash is taken into account.
            double CDBa=cdbbody(d,l,ln,Mach,abs(atr));
            double CDiTB=CDLT*St/Sref+CDBa;
            double CDLW=cdlwing(bw,Sw,taperw,Mach,atr+dtr);
            double CDW=CDoW+CDLW;
            double CLd=cldwbt_ndw(d,bw,Sw,taperw,Mach,isTailControlled)*Sref/Sw;
            double CLW=CLd*abs(dtr);
            double nw=1;
            double CDWR=(CDW*cos(dtr)+CLW*sin(dtr))*Sw*nw/Sref;
            return CDoTB+CDiTB+CDWR;
        }
    }
    double cfturbfp(double chardim,double Mach,double alt){
        std::map<std::string,double> atm=atmosphere(alt);
        double Rn=chardim*Mach*atm["a"]/atm["nu"];
        double cfi=0.455/pow(log10(Rn),2.58);
        double cf;
        if(Mach<=1){
            cf=cfi/(1+0.08*Mach*Mach);
        }else{
            cf=cfi/pow(1+0.144*Mach*Mach,0.65);
        }
        return cf;
    }
    std::pair<double,double> clacma(double d,double l,double ln,double lcg,double bw,double Sw,double taperw,double lw,double bt,double St,double tapert,double Mach,double alpha){
        //Combined CLAMSLSB, CLAMSLSP, CLAWBT, CMASUB, CMASUP, and CMAWBT in a single function
        double Sref=M_PI*d*d/4.;
        //Body
        double CLaN=bdycla(d,l,ln,Mach,alpha);
        double cpn,xn;
        if(Mach<1){
            cpn=cpogvsb(ln,d);
        }else{
            cpn=cpogvemp(d,ln,Mach);
        }
        xn=lcg-cpn;
        double CMaNd=xn*CLaN;
        //Wing
        double Aw=bw*bw/Sw;
        double sw=0.5*(d+bw);
        double crw=2*Sw/bw/(1+taperw);
        double CLaW,CLaW0,Kbw,Kwb,xbw,xwb;
        if(Mach<1){
            double lc2w=atan(2/Aw*(1-taperw)/(1+taperw));
            double kappaw=0.85;
            CLaW=clawsub(Mach,Aw,taperw,lc2w,kappaw);
            CLaW0=clawsub(0,Aw,taperw,lc2w,kappaw);
            Kbw=bigkbwsb(d/2,sw);
            Kwb=bigkwb(d/2,sw);
            xbw=lw+xcrbwsub(Mach,Aw,d/2,sw,taperw)*crw-lcg;
            xwb=lw+xcrwbsub(Mach,Aw,taperw)*crw-lcg;
        }else{
            CLaW=clawsup(Mach,Aw,taperw);
            Kbw=bigkbwa(Mach,Aw,d/2,sw,taperw);
            Kwb=bigkwb(d/2,sw);
            double betaw=sqrt(Mach*Mach-1);
            double xcrwb;
            if(taperw==0.0){
                xcrwb=xcrwba(d/2,sw);
            }else{
                xcrwb=xcrwbsup(Mach,Aw,taperw);
            }
            double xcrbw=xcrbwsupa(Mach,Aw,d/2,sw,taperw);
            xbw=lw+xcrbw*crw-lcg;
            xwb=lw+xcrwb*crw-lcg;
        }
        //Tail
        double Mt=0.95*Mach;
        double nt=pow(Mt/Mach,2);
        double At=bt*bt/St;
        double st=0.5*(d+bt);
        double crt=2*St/bt/(1+tapert);
        double CLaT,Kbt,Ktb,xbt,xtb;
        if(Mt<1){
            double lc2t=atan(2/At*(1-tapert)/(1+tapert));
            double kappat=0.85;
            CLaT=clawsub(Mt,At,tapert,lc2t,kappat);
            Kbt=bigkbwsb(d/2,st);
            Ktb=bigkwb(d/2,st);
        }else{
            CLaT=clawsup(Mt,At,tapert);
            Kbt=bigkbwna(Mt,At,d/2,st,tapert);
            Ktb=bigkwb(d/2,st);
        }
        if(Mt<1){
            double lt=l-crt;
            xbt=lt+xcrbwsub(Mt,At,d/2,st,tapert)*crt-lcg;
            xtb=lt+xcrwbsub(Mt,At,tapert)*crt-lcg;
        }else{
            double lt=l-crt;
            double betat=sqrt(abs(Mt*Mt-1));
            double xcrtba;
            if(tapert==0){
                xcrtba=xcrwba(d/2,st);
            }else{
                xcrtba=xcrwbsup(Mt,At,tapert);
            }
            double xcrbta=xcrbwsupna(Mt,At,d/2,st,tapert);
            xbt=lt*xcrbta*crt-lcg;
            xtb=lt*xcrtba*crt-lcg;
        }
        //Downwash
        double deda;
        if(Mach<1){
            double lH=l-St/bt*(1+tapert*tapert/(1+tapert))/(1+tapert)-(lw+Sw/bw*(1-taperw*taperw/(1+taperw))/(1+taperw));
            double lc4w=atan(3/Aw*(1-taperw)/(1+taperw));
            deda=dedasub(Aw,bw,lc4w,taperw,lH,0)*CLaW/CLaW0;
        }else{
            double SWPwing=atan(2*crw/bw*(1-taperw));
            double lt=l-crt;
            double x0=(lt-(lw+crw))/crw;
            deda=Kwb*dedasup(Mach,SWPwing,x0);
        }
        double cla=CLaN+(Kbw+Kwb)*CLaW*Sw/Sref+(Kbt+Ktb)*CLaT*(1-deda)*nt*St/Sref;
        double cmad=CMaNd-(Kbw*xbw+Kwb*xwb)*CLaW*Sw/Sref-(Kbt*xbt+Ktb*xtb)*CLaT*(1-deda)*nt*St/Sref;
        return std::make_pair(cla,cmad/d);
    }
    double clawsub(double M,double A_,double taper,double lc2,double kappa){
        double b=sqrt(abs(M*M-1));
        double A=A_*(1+taper)/(1-taper);
        return 2*M_PI*A/(2+sqrt(pow(A*b/kappa,2)*(1+pow(tan(lc2)/b,2))+4));
    }
    double clawsup(double M,double A,double taper){
        double b=sqrt(abs(M*M-1));
        double m=A/4*(1+taper)/(1-taper);
        if(m*b>=1){
            return 4/b;
        }else{
            double k=sqrt(1-pow(m*b,2));
            if(std::isnan(k)){
                // 0<=m*b<1なのでk>0になるはずだが、
                // 浮動小数点数の誤差によりごく稀にkがNaNになる。
                // その場合はmを0とみなしてE=π/2を得る。
                k=0;
            }
            double E=boost::math::ellint_2(k);
            return 2*M_PI*m/E;
        }
    }
    std::pair<double,double> cldcmd(double d,double l,double ln,double lcg,double bw,double Sw,double taperw,double lw,double bt,double St,double tapert,double Mach, bool isTailControlled){
        //Combined CLDWBT and CMDWBT in a single function
        double Mt=0.95*Mach;
        double nt=pow(Mt/Mach,2);
        double Sref=M_PI*d*d/4;
        double At=bt*bt/St;
        double st=0.5*(d+bt);
        double crt=2*St/bt/(1+tapert);
        double lt=l-crt;
        if(isTailControlled){
            double kbt=smlkbw(d/2,st);
            double ktb=smlkwb(d/2,st);
            double CLaT,xcrbt,xcrtb;
            if(Mt<1){
                double lc2t=atan(2/At*(1-tapert)/(1+tapert));
                double kappat=0.85;
                CLaT=clawsub(Mt,At,tapert,lc2t,kappat);
                xcrbt=xcrbwsub(Mt,At,d/2,st,tapert);
                xcrtb=xcrwbsub(Mt,At,tapert);
            }else{
                CLaT=clawsup(Mt,At,tapert);
                xcrbt=xcrbwsupna(Mt,At,d/2,st,tapert);
                if(tapert==0){
                    xcrtb=xcrwbd(d/2,st);
                }else{
                    xcrtb=xcrwbsup(Mt,At,tapert);
                }
            }
            double lbt=lt+xcrbt*crt;
            double ltb=lt+xcrtb*crt;
            double xbt=lbt-lcg;
            double xtb=ltb-lcg;
            double cld=CLaT*nt*(kbt+ktb)*St/Sref;
            double cmd=-CLaT*nt*(kbt*xbt+ktb*xtb)*St/Sref/d;
            return std::make_pair(cld,cmd);
        }else{
            double Aw=bw*bw/Sw;
            double sw=0.5*(d+bw);
            double crw=2*Sw/bw/(1+taperw);
            double kbw=smlkbw(d/2,sw);
            double kwb=smlkwb(d/2,sw);
            double CLaW,CLaW0,xcrbw,xcrwb;
            if(Mach<1){
                double lc2w=atan(2/Aw*(1-taperw)/(1+taperw));
                double kappaw=0.85;
                CLaW=clawsub(Mach,Aw,taperw,lc2w,kappaw);
                CLaW0=clawsub(0,Aw,taperw,lc2w,kappaw);
                xcrbw=xcrbwsub(Mach,Aw,d/2,sw,taperw);
                xcrwb=xcrwbsub(Mach,Aw,taperw);
            }else{
                CLaW=clawsup(Mach,Aw,taperw);
                xcrbw=xcrbwsupna(Mach,Aw,d/2,sw,taperw);
                if(taperw==0){
                    xcrwb=xcrwbd(d/2,sw);
                }else{
                    xcrwb=xcrwbsup(Mach,Aw,taperw);
                }
            }
            double lbw=lw+xcrbw*crw;
            double lwb=lw+xcrwb*crw;
            double xbw=lbw-lcg;
            double xwb=lwb-lcg;
            double cld=CLaW*(kbw+kwb)*Sw/Sref;
            double cmd=-CLaW*(kbw*xbw+kwb*xwb)*Sw/Sref/d;

            //Downwash by deflection
            double CLaT,Kbt,Ktb,xbt,xtb;
            if(Mt<1){
                double lc2t=atan(2/At*(1-tapert)/(1+tapert));
                double kappat=0.85;
                CLaT=clawsub(Mt,At,tapert,lc2t,kappat);
                Kbt=bigkbwsb(d/2,st);
                Ktb=bigkwb(d/2,st);
            }else{
                CLaT=clawsup(Mt,At,tapert);
                Kbt=bigkbwna(Mt,At,d/2,st,tapert);
                Ktb=bigkwb(d/2,st);
            }
            if(Mt<1){
                double lt=l-crt;
                xbt=lt+xcrbwsub(Mt,At,d/2,st,tapert)*crt-lcg;
                xtb=lt+xcrwbsub(Mt,At,tapert)*crt-lcg;
            }else{
                double lt=l-crt;
                double betat=sqrt(abs(Mt*Mt-1));
                double xcrtba;
                if(tapert==0){
                    xcrtba=xcrwba(d/2,st);
                }else{
                    xcrtba=xcrwbsup(Mt,At,tapert);
                }
                double xcrbta=xcrbwsupna(Mt,At,d/2,st,tapert);
                xbt=lt*xcrbta*crt-lcg;
                xtb=lt*xcrtba*crt-lcg;
            }
            double deda;
            if(Mach<1){
                double lH=l-St/bt*(1+tapert*tapert/(1+tapert))/(1+tapert)-(lw+Sw/bw*(1-taperw*taperw/(1+taperw))/(1+taperw));
                double lc4w=atan(3/Aw*(1-taperw)/(1+taperw));
                deda=dedasub(Aw,bw,lc4w,taperw,lH,0)*CLaW/CLaW0;
            }else{
                double SWPwing=atan(2*crw/bw*(1-taperw));
                double lt=l-crt;
                double x0=(lt-(lw+crw))/crw;
                double Kwb=bigkwb(d/2,sw);
                deda=Kwb*dedasup(Mach,SWPwing,x0);
            }
            cld=cld+(Kbt+Ktb)*CLaT*(-deda)*nt*St/Sref;
            cmd=cmd-(Kbt*xbt+Ktb*xtb)*CLaT*(deda)*nt*St/Sref;

            return std::make_pair(cld,cmd);
        }
    }
    double cldwbt_ndw(double d,double b,double S,double taper,double Mach, bool isTailControlled){
        double Sref=M_PI*d*d/4;
        double A=b*b/S;
        double s=0.5*(d+b);
        double M,n;
        if(isTailControlled){
            M=0.95*Mach;
            n=pow(M/Mach,2);
        }else{
            M=Mach;
            n=1;
        }
        double CLa;
        if(M<1){
            double lc2=atan(2/A*(1-taper)/(1-taper));
            double kappa=0.85;
            CLa=clawsub(M,A,taper,lc2,kappa);
        }else{
            CLa=clawsup(M,A,taper);
        }
        double kbw=smlkbw(d/2,s);
        double kwb=smlkwb(d/2,s);
        return CLa*n*(kbw+kwb)*S/Sref;
    }
    double cpogvemp(double d,double ln,double M){
        double sigma=2*atan(d/2/ln)*180/M_PI;
        double P=(0.083+0.096/pow(M,2))*pow(sigma/10,1.69);
        return 0.5*(50*(M+18)+7*M*M*P*(5*M-18))*ln/(40*(M+18)+7*M*M*P*(4*M-3));
    }
    double cpogvsb(double ln,double d){
        double Vn=vologv1(ln,d);
        return ln-4*Vn/M_PI/pow(d,2);
    }
    double dedasub(double A,double b,double lc4,double lam,double lH,double hH){
        double KA=1./A-1./(1.+pow(A,1.7));
        double Kl=(10.-3.*lam)/7.;
        double KH=(1-abs(hH/b))/pow(2*lH/b,1./3.);
        return 4.44*pow(KA*Kl*KH*sqrt(cos(lc4)),1.19);
    }
    double dedasup(double M,double lam,double x0){
        double psi=M_PI/2-lam;
        double t0=tan(psi)*sqrt(M*M-1);
        double e2,e4,e6,e8;
        if(x0<1.0){
            double e2=0.81;
        }else if(x0<=1.3){
            static const Eigen::VectorXd p21{{
                -1.666666666666488e0,
                5.999999999999325e0,
                -6.783333333332485e0,
                3.259999999999646e0
            }};
            e2=polyval(p21,x0);
        }else if(x0<2.3){
            static const Eigen::VectorXd p22{{
                -1.279239755663028e2,
                1.843272381677282e3,
                -1.155286798315866e4,
                4.113252460613160e4,
                -9.098009805564612e4,
                1.280057606805492e5,
                -1.118657697776747e5,
                5.551390292092566e4,
                -1.197584408390502e4
            }};
            e2=polyval(p22,x0);
        }else{
            e2=0.97;
        }
        if(x0<1.0){
            e4=0.65;
        }else if(x0<=1.6){
            static const Eigen::VectorXd p41{{
                -4.166666666665245e0,
                2.083333333332625e1,
                -3.845833333332031e1,
                3.134166666665624e1,
                -8.899999999996956e0
            }};
            e4=polyval(p41,x0);
        }else if(x0<=3.0){
            static const Eigen::VectorXd p42{{
                4.475474425067136e0,
                -8.198016093532951e1,
                6.529374943388477e2,
                -2.952920552556513e3,
                8.292996506025491e3,
                -1.480813974021849e4,
                1.641631929076350e4,
                -1.032958858524409e4,
                2.825103492654550e3
            }};
            e4=polyval(p42,x0);
        }else{
            e4=0.87;
        }
        if(x0<1.0){
            e6=0.54;
        }else if(x0<=1.4){
            static const Eigen::VectorXd p61{{
                1.600881353985564e-14,
                -6.387361790702744e-14,
                1.000000000000832e-1,
                4.399999999999644e-1
            }};
            e6=polyval(p61,x0);
        }else if(x0<=1.8){
            static const Eigen::VectorXd p62{{
                -3.333333333309070e1,
                2.116666666651215e2,
                -5.026666666629868e2,
                5.294833333294496e2,
                -2.082299999984673e2
            }};
            e6=polyval(p62,x0);
        }else if(x0<3.6){
            static const Eigen::VectorXd p63{{
                6.356470889426016e-3,
                -5.240723295023863e-2,
                -2.580914111953288e-2,
                1.556718856871693e0,
                -6.218769329470563e0,
                1.005756492417748e1,
                -5.269710040962651e0
            }};
            e6=polyval(p63,x0);
        }else{
            e6=0.79;
        }
        if(x0<1.0){
            e8=0.44;
        }else if(x0<=1.4){
            static const Eigen::VectorXd p81{{
                -1.666666666665405e1,
                7.999999999993737e1,
                -1.433333333332168e2,
                1.136499999999038e2,
                -3.320999999997028e1
            }};
            e8=polyval(p81,x0);
        }else if(x0<=2.2){
            static const Eigen::VectorXd p82{{
                -8.680554787360018e2,
                1.252976079851500e4,
                -7.886457638672026e4,
                2.827083084908162e5,
                -6.312798925797358e5,
                8.991348588842052e5,
                -7.977027436345048e5,
                4.030393822169418e5,
                -8.878791230326184e4
            }};
            e8=polyval(p82,x0);
        }else if(x0<3.4){
            static const Eigen::VectorXd p83{{
                3.866380123250028e0,
                -9.169392757855618e1,
                9.469642944017384e2,
                -5.562064279886374e3,
                2.032063141263390e4,
                -4.728372988378842e4,
                6.842861819191089e4,
                -5.630916695234881e4,
                2.017213892626900e4
            }};
            e8=polyval(p83,x0);
        }else{
            e8=0.7;
        }
        double dedaw;
        if(t0<=0.2){
            dedaw=(e2-e4)*(0.2-t0)/0.2+e2;
        }else if(t0<=0.4){
            dedaw=(e2-e4)*(0.4-t0)/0.2+e4;
        }else if(t0<=0.6){
            dedaw=(e4-e6)*(0.6-t0)/0.2+e6;
        }else if(t0<=0.8){
            dedaw=(e6-e8)*(0.8-t0)/0.2+e8;
        }else{//if(t0>0.8){
            dedaw=e8-(e6-e8)*(t0-0.8)/0.2;
        }
        return std::max(0.0,dedaw);
    }
    double dragfctr(double fineness){
        static const Eigen::VectorXd p{{
            -3.610649936485454e-12,
            8.309751611041349e-10,
            -7.347789500313429e-8,
            3.037472628776377e-6,
            -5.190776944854130e-5,
            -1.773312264112830e-4,
            2.135202437974324e-2,
            5.187105185342232e-1
        }};
        return polyval(p,fineness);
    }
    double k2mk1(double fineness){
        static const Eigen::VectorXd pa{{
            -6.3424e-7,
            3.1805e-5,
            -6.0351e-4,
            5.0080e-3,
            -9.9444e-3,
            -1.1439e-1,
            7.7981e-1,
            -6.6077e-1
        }};
        static const Eigen::VectorXd pb{{1.6667e-3,9.4667e-1}};
        if(fineness<0){
            fineness=0;
        }
        double amf;
        if(fineness<14){
            amf=polyval(pa,fineness);
        }else{
            amf=polyval(pb,fineness);
        }
        return std::max(0.0,amf);
    }
    double smlkbw(double r,double s){
        return bigkwb(r,s)-smlkwb(r,s);
    }
    double smlkwb(double r,double s){
        double t=s/r;
        double kwb;
        if(t<=1){
            kwb=1;
        }else{
            kwb=pow(M_PI*(t+1)/t,2)/4+M_PI*pow((t*t+1)/(t*(t-1)),2)*asin((t*t-1)/(t*t+1));
            kwb=kwb-2*M_PI*(t+1)/t/(t-1)+pow((t*t+1)/(t*(t-1)),2)*pow(asin((t*t-1)/(t*t+1)),2);
            kwb=kwb-4*(t+1)*asin((t*t-1)/(t*t+1))/t/(t-1)+8*log((t*t+1)/2/t)/pow(t-1,2);
            kwb=kwb/pow(M_PI,2);
        }
        return kwb;
    }
    double sscfdccc(double alpha,double M){
        static const Eigen::VectorXd pa{{-2.5513,4.9961,-1.5000,0.1394,1.1967}};
        static const Eigen::VectorXd pb{{-2.1943e2,9.0057e2,-1.4619e3,1.1695e3,-4.5889e2,7.1919e1}};
        static const Eigen::VectorXd pc{{154.7931,-553.2488,737.6712,-434.1301,96.7047}};
        static const Eigen::VectorXd pd{{-3.4916,7.9077,-6.1199,2.3609,-7.8519e-2,1.2400}};
        double k=sin(alpha)*M;
        if(k>1){
            k=1/k;
            if(k>0.8){
                return polyval(pc,k);
            }else{
                return polyval(pd,k);
            }
        }else{
            if(k>0.6){
                return polyval(pb,k);
            }else{
                return polyval(pa,k);
            }
        }
    }
    double surfogv1(double diameter,double length){
        double ldr=length/diameter;
        double ldr2=ldr*ldr;
        double surf1=asin(ldr/(ldr2+0.25));
        return M_PI*pow(diameter,2)*(pow(ldr2+0.25,2)*surf1-ldr*(ldr2-0.25));
    }
    double vologv1(double logv,double dbase){
        double l=logv;
        double d=dbase;
        double R=d/4+l*l/d;
        double vol=(R-d/2)*(l*sqrt(R*R-l*l)+R*R*asin(l/R));
        vol=M_PI*(l*(2*R*R-R*d+d*d/4)-pow(l,3)/3-vol);
        return vol;
    }
    double wvdrgogv(double diameter,double length,double Mach){
        double sigma=2*(180/M_PI)*atan(diameter/2/length);
        double P=(0.083+0.096/(Mach*Mach))*pow(sigma/10,1.69);
        double lod2=pow(length/diameter,2);
        return P*(1-(392*lod2-32)/28/lod2/(Mach+18));
    }
    /*
    double xcrbwabh(double M,double r,double cr){
        //supersonic, with afterbody and low aspect ratio
        double b=sqrt(abs(M*M-1));
        double t=2*b*r/cr;
        static const Eigen::VectorXd p{{1.7611e-2,-1.0474e-1,5.9054e-1,5.0298e-1}};
        return polyval(p,t);
    }
    double xcrbwabl(double M,double A,double r,double s){
        //supersonic, with afterbody and high aspect ratio
        double b=sqrt(abs(M*M-1));
        static const Eigen::VectorXd p{{1.7976e0,-8.2857e-1,4.3667e-1,-5.7381e-17}};
        double t1=b*A;
        double t2=r/s;
        double slope=polyval(p,t2);
        return slope*t1+0.5;
    }
    double xcrbwnab(double M,double r,double cr){
        //supersonic and without afterbody
        double b=sqrt(abs(M*M-1));
        double t=2*b*r/cr;
        static const Eigen::VectorXd p{{8.3600e0,-3.0112e1,4.3283e1,-3.1734e1,1.2746e1,-3.0908e0,7.1424e-1,5.0019e-1}};
        if(t>0.9){
            return 2./3.;
        }else{
            return polyval(p,t);
        }
    }
    */
    double xcrbwsub(double M,double A,double r,double s,double taper){
        //subsonic
        //NACA TR 1307 Eq. (D6)
        double b=sqrt(abs(1-M*M));
        double t=r/s;
        double xcr=(sqrt(1-2*t)*acosh((1-t)/t)-(1-t)+M_PI*t/2)/((1-t)*t/sqrt(1-2*t)*acosh((1-t)/t)+pow(1-t,2)/t-M_PI*(1-t)/2);
        return 0.25+3/2*(1-taper)*(t/(t-1)+xcr);
        /*double rs6,rs4,rs2,rs0;
        if(b*A<4){
            static const Eigen::VectorXd p6{{
                -1.503003360588808e-4,
                1.500533636756856e-3,
                -5.843498893035511e-3,
                1.266301279846288e-2,
                4.997093223254211e-1
            }};
            rs6=polyval(p6,b*A);
            static const Eigen::VectorXd p4{{
                1.439562405737517e-4,
                -2.577574370693720e-3,
                1.915260292425828e-2,
                -7.636557567970656e-2,
                1.764081283046741e-1,
                -2.379409374985050e-1,
                1.805435508411277e-1,
                -7.176958947911805e-2,
                5.130816740444732e-3,
                4.999682326019724e-1
            }};
            rs4=polyval(p4,b*A);
            static const Eigen::VectorXd p2{{
                1.870466827291419e-4,
                -3.055345401484121e-3,
                2.012956338935916e-2,
                -6.808482446989989e-2,
                1.241295486321807e-1,
                -1.160715264335058e-1,
                4.940441636896099e-2,
                -4.087742141651518e-2,
                5.000384633792437e-1
            }};
            rs2=polyval(p2,b*A);
        }else{
            rs6=0.5+2.6*0.05/9;
            rs4=0.45+4*0.05/9;
            rs2=0.4+1.5*0.05/9;
        }
        if(b*A>=7){
            rs0=0.25;
        }else{
            static const Eigen::VectorXd p0{{
                1.368278964039268e-5,
                -2.870181449710582e-4,
                2.101837326902790e-3,
                -6.106506978595964e-3,
                1.093840736149155e-2,
                -7.485137721564938e-2,
                5.003830336176011e-1
            }};
            rs0=polyval(p0,b*A);
        }
        if(t>0.6){
            return rs6+(rs6-rs4)*(t-0.6)/0.2;
        }else if(t>0.4){
            return rs4+(rs6-rs4)*(t-0.4)/0.2;
        }else if(t>0.2){
            return rs2+(rs4-rs2)*(t-0.2)/0.2;
        }else{
            return rs0+(rs2-rs0)*t/0.2;
        }
        */
    }
    double xcrbwsupa(double M,double A,double r,double s,double taper){
        //NACA TR 1307 Eq. (66),(68),(79)
        double b=sqrt(abs(M*M-1));
        double cr=4*(s-r)/A/(1+taper);
        double m=A/4*(1+taper)/(1-taper);
        double mb=m*b;
        double dcr=2*r/cr;
        double bdcr=b*dcr;
        if(mb>1){
            //supersonic leading edge Eq. (66),(68)
            double ret=sqrt(1+2*bdcr)*((2*mb+5)/pow(mb+1,2)/3+bdcr/(mb+1)/3-bdcr*bdcr/mb)-(2*mb+5)/pow(mb+1,2)/3;
            ret=ret+(pow(1+bdcr,3)-pow(bdcr,3)/pow(mb,2)-1/pow(mb+1,2))*acos((1+bdcr*(mb+1))/(mb+bdcr*(mb+1)))/sqrt(mb*mb-1);
            ret=ret+pow(bdcr,3)/pow(mb,2)*acosh(1+1/bdcr)-(1-1/pow(mb+1,2))*acos(1/mb)/sqrt(mb*mb-1);
            double Kbw=bkbwspa(M,A,r,s,taper);
            double CLaW=clawsup(M,A,taper);
            return ret*4/(3*M_PI*b*(1+taper)*(1-taper)*Kbw*CLaW);
        }else{
            //subsonic leading edge Eq. (68),(69)
            double ret=sqrt(m*m+m*(mb+1)*dcr)/(9*m*pow(mb+1,3))*((8*mb+24)*m*m+(14*mb+6)*(mb+1)*m*dcr+3*(mb-3)*pow(mb+1,2)*dcr*dcr);
            ret=ret-(8*mb+24)*pow(m,3)/(9*m*pow(mb+1,3))-(mb-3)/(3*m)*pow(dcr,3)*acosh(sqrt((m+(mb+1)*dcr)/((mb+1)*dcr)));
            ret=ret*4/(M_PI*m*(1+taper)*(1-taper));
            double Kbw=bkbwsba(M,A,r,s,taper);
            double CLaW=clawsup(M,A,taper);
            return ret/(Kbw*CLaW);
        }
    }
    double xcrbwsupna(double M,double A,double r,double s,double taper){
        //NACA TR 1307 Eq. (67),(68),(70)
        double b=sqrt(abs(M*M-1));
        double cr=4*(s-r)/A/(1+taper);
        double m=A/4*(1+taper)/(1-taper);
        double mb=m*b;
        double dcr=2*r/cr;
        double bdcr=b*dcr;
        double ret;
        if(mb>1){
            //supersonic leading edge Eq. (67),(68)
            if(bdcr<1){
                ret=-pow(bdcr,2)/mb*sqrt(1-pow(bdcr,2))+2*asin(bdcr)+pow(bdcr,3)/pow(mb,2)*acosh(1/bdcr);
                ret=ret+(-pow(mb,3)+3*mb*mb*bdcr-pow(bdcr,3))/pow(mb,2)/sqrt(mb*mb-1)*acos((1+mb*bdcr)/(mb+bdcr));
                ret=ret+mb/sqrt(mb*mb-1)*(acos(1/mb)+3*atan(sqrt((mb*mb-1)*(1-bdcr*bdcr))/(1+mb*bdcr))-3*atan(sqrt(mb*mb-1)));
            }else{
                ret=M_PI;
                ret=ret+mb/sqrt(mb*mb-1)*(acos(1/mb)-3*atan(sqrt(mb*mb-1)));
            }
            double Kbw=bkbwspna(M,A,r,s,taper);
            double CLaW=clawsup(M,A,taper);
            return ret*16*m/(3*M_PI*pow(1+taper,2)*A*b*Kbw*CLaW);
        }else{
            //subsonic leading edge Eq. (68),(70)
            if(bdcr<1){
                ret=sqrt(m*(1-bdcr)*(m+dcr))/(3*m*(mb+1))*(4*m*m+2*m*dcr-(3+mb)*dcr*dcr);
                ret=ret-4*m*m/(3*(mb+1))-(mb-3)/(3*m)*pow(dcr,3)*acosh(sqrt((m+dcr)/(mb+1)/dcr));
                if(b<1e-3){
                    ret=ret+4*m*m/3*(sqrt((1+dcr/m)/(1-bdcr))*atanx_x(sqrt((mb+bdcr)/(1-bdcr)))-atanx_x(sqrt(mb)));
                }else{
                    ret=ret+4*m/b*sqrt(mb)/3*(atan2(sqrt(mb+bdcr),sqrt(1-bdcr))-atan(sqrt(mb)));
                }
            }else{
                ret=-4*m*m/(3*(mb+1))+4*m*sqrt(mb)/3/b*(M_PI_2-atan(sqrt(mb)));
            }
            double Kbw=bkbwsbna(M,A,r,s,taper);
            double CLaW=clawsup(M,A,taper);
            return ret*16*m/(M_PI*pow(1+taper,2)*A*Kbw*CLaW);
        }
    }
    /*
    double xcrw(double M,double A){
        double b=sqrt(abs(M*M-1));
        double mb=b*A;
        return 2./3.;
    }
    */
    double xcrwba(double r,double s){
        //NACA TR 1307 Chart 13
        double t=r/s;
        static const Eigen::VectorXd p{{-0.0822,0.2009,-0.1190,0.6667}};
        return polyval(p,t);
    }
    double xcrwbd(double r,double s){
        //NACA TR 1307 Chart 13
        double t=r/s;
        static const Eigen::VectorXd p{{0.8716,-3.0894,4.3106,-3.1344,1.3829,-0.3885,0.0473,0.6669}};
        return polyval(p,t);
    }
    double xcrwbsub(double M,double A,double taper){
        //NACA TR 1307 Chart 11(c)
        double b=sqrt(abs(1-M*M));
        std::array<Eigen::VectorXd,1> xp{
            Eigen::VectorXd{{0.0,0.5,1.0,1.5,2.0,4.0,6.0,8.0}}
        };
        Eigen::Tensor<double,1,Eigen::RowMajor> y0=Eigen::Tensor<double,1,Eigen::RowMajor>(8).setValues({
            0.655,0.600,0.572,0.561,0.555,0.544,0.539,0.544
        });
        Eigen::Tensor<double,1,Eigen::RowMajor> y0p5=Eigen::Tensor<double,1,Eigen::RowMajor>(8).setValues({
            0.333,0.366,0.394,0.405,0.411,0.411,0.411,0.411
        });
        Eigen::Tensor<double,1,Eigen::RowMajor> y1=Eigen::Tensor<double,1,Eigen::RowMajor>(8).setValues({
            0.0,0.161,0.222,0.244,0.250,0.250,0.250,0.250
        });
        double x=b*A;
        double f0=interpn(xp,y0,x);
        double f0p5=interpn(xp,y0p5,x);
        double f1=interpn(xp,y1,x);
        if(taper<0.5){
            return f0+(f0p5-f0)*taper/0.5;
        }else{
            return f0p5+(f1-f0p5)*(taper-0.5)/0.5;
        }
        /*if(b*A<2){
            static const Eigen::VectorXd p{{
                -1.130484330484368e-2,
                5.420357420357615e-2,
                -1.073193473193506e-1,
                1.446257446257462e-1,
                -1.627277907277905e-1,
                6.555089355089354e-1
            }};
            return polyval(p,b*A);
        }else if(b*A>=5){
            return 0.5+3.5/90;
        }else{
            static const Eigen::VectorXd p{{
                -1.785830762908004e-17,-5.555555555555476e-3,5.666666666666665e-1
            }};
            return polyval(p,b*A);
        }*/
    }
    double xcrwbsup(double M,double A,double taper){
        //NACA TR 1307 Chart 10(c)
        double b=sqrt(abs(1-M*M));
        Eigen::Tensor<double,1,Eigen::RowMajor> y0p5=Eigen::Tensor<double,1,Eigen::RowMajor>(7).setValues({
            0.340,0.450,0.510,0.570,0.590,0.600,0.610
        });
        Eigen::Tensor<double,1,Eigen::RowMajor> y1=Eigen::Tensor<double,1,Eigen::RowMajor>(8).setValues({
            0.0,0.2,0.345,0.420,0.450,0.470,0.480,0.495
        });
        double x=b*A;
        double f0=2/3;
        std::array<Eigen::VectorXd,1> xp{
            Eigen::VectorXd{{0.0,0.5,1.0,2.0,3.0,5.0,8.0}}
        };
        double f0p5=interpn(xp,y0p5,x);
        xp[0]=Eigen::VectorXd{{0.0,0.5,1.0,1.5,2.0,3.0,4.0,8.0}};
        double f1=interpn(xp,y1,x);
        if(taper<0.5){
            return f0+(f0p5-f0)*taper/0.5;
        }else{
            return f0p5+(f1-f0p5)*(taper-0.5)/0.5;
        }
    }
}

void exportEkkerMDTV2(py::module &m_){
    using namespace EkkerMDTV2;

	auto m=m_.def_submodule("EkkerMDTV2");
    m.def("areapogv",&areapogv);
    m.def("bdycla",&bdycla);
    m.def("bigkbwa",&bigkbwa);
    m.def("bigkbwna",&bigkbwna);
    m.def("bigkbwsb",&bigkbwsb);
    m.def("bigkwb",&bigkwb);
    m.def("bkbwsba",&bkbwsba);
    m.def("bkbwsbna",&bkbwsbna);
    m.def("bkbwspa",&bkbwspa);
    m.def("bkbwspna",&bkbwspna);
    m.def("bsdrgcf",&bsdrgcf);
    m.def("bsdrgsp",&bsdrgsp);
    m.def("cdbbody",&cdbbody);
    m.def("cdlcomp",&cdlcomp);
    m.def("cdlwing",&cdlwing);
    m.def("cdobody",&cdobody);
    m.def("cdowbt",&cdowbt);
    m.def("cdowing",&cdowing);
    m.def("cdtrim",&cdtrim);
    m.def("cfturbfp",&cfturbfp);
    m.def("clacma",&clacma);
    m.def("clawsub",&clawsub);
    m.def("clawsup",&clawsup);
    m.def("cldcmd",&cldcmd);
    m.def("cldwbt_ndw",&cldwbt_ndw);
    m.def("cpogvemp",&cpogvemp);
    m.def("cpogvsb",&cpogvsb);
    m.def("dedasub",&dedasub);
    m.def("dedasup",&dedasup);
    m.def("dragfctr",&dragfctr);
    m.def("k2mk1",&k2mk1);
    m.def("smlkbw",&smlkbw);
    m.def("smlkwb",&smlkwb);
    m.def("sscfdccc",&sscfdccc);
    m.def("surfogv1",&surfogv1);
    m.def("vologv1",&vologv1);
    m.def("wvdrgogv",&wvdrgogv);
    //m.def("xcrbwabh",&xcrbwabh);
    //m.def("xcrbwabl",&xcrbwabl);
    //m.def("xcrbwnab",&xcrbwnab);
    m.def("xcrbwsub",&xcrbwsub);
    m.def("xcrbwsupa",&xcrbwsupa);
    m.def("xcrbwsupna",&xcrbwsupna);
    //m.def("xcrw",&xcrw);
    m.def("xcrwba",&xcrwba);
    m.def("xcrwbd",&xcrwbd);
    m.def("xcrwbsub",&xcrwbsub);
    m.def("xcrwbsup",&xcrwbsup);
}

ASRC_NAMESPACE_END(core)
ASRC_NAMESPACE_END(asrc)
