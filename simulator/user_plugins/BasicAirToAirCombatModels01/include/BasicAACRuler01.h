// Copyright (c) 2021-2026 Air Systems Research Center, Acquisition, Technology & Logistics Agency(ATLA)
#pragma once
#include "Common.h"
#include <vector>
#include <map>
#include <regex>
#include <pybind11/pybind11.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <ASRCAISim1/MathUtility.h>
#include <ASRCAISim1/Utility.h>
#include <ASRCAISim1/Ruler.h>
#include <ASRCAISim1/PhysicalAsset.h>
namespace py=pybind11;
namespace nl=nlohmann;

ASRC_PLUGIN_NAMESPACE_BEGIN

/**
 * @brief つねに値型としてシリアライゼーションを行うタイプのうち、多態的な振る舞いが必要な場合に用いる基底クラス
 * 
 * @details
    基本的な空対空戦闘のルールを表現したクラス。得点は陣営(team)単位。
    基本仕様は第2回空戦AIチャレンジ以降変更しておらず、都度設定可能項目を追加したものである。

    # 戦域について
	    - 座標系はNEDで、地表面をz=0とする。戦域の中心はx=y=0とする。
	    - 戦域の中心から東西(y軸)方向に±dLine[m]の位置に引かれた直線を各々の防衛ラインとする。
	    - 南北(x軸)方向は±dOut[m]までを戦域とする。

        ## 帰還可能性の判定について

            modelConfigにおいて`considerFuelConsumption`フラグをtrueとした場合、各機の燃料消費を考慮する。

            燃料消費を考慮する場合、終了時に生存していた機体は自陣営の基地に帰還するために必要な燃料を残しているかどうかを判定され、
            残燃料が不十分だった機体は帰還不可として扱い、得点計算においては墜落したものとみなす。

            各陣営の防衛ラインから自陣営までの距離は、modelConfigにおいて`distanceFromBase`として陣営名をキーとしたdictで与える。

    # 勝敗に関わるイベントの判定対象について

        ## 全滅判定対象機

            全滅について、各機のFactoryモデル名で判定対象かどうかを区別する。
            
            `modelNamesToBeConsideredForElimination`として判定対象に含めるモデル名の正規表現パターンをリストで与え、
            マッチした全機体を対象に加える。また、"Any"を予約語とし、"Any"が与えられた場合は全機体を対象に加える。

            また、`modelNamesToBeExcludedForElimination`として判定対象から除外するモデル名の正規表現パターンをリストで与え、
            マッチした全機体を対象から除外する。

            両方にマッチした場合は除外を優先する。

            modelConfigの関係する項目は以下のとおり。
            - modelNamesToBeConsideredForElimination (list[str])
            - modelNamesToBeExcludedForElimination (list[str])

        ## 突破判定対象機

            防衛ラインの突破について、各機のFactoryモデル名で判定対象かどうかを区別する。
            
            `modelNamesToBeConsideredForBreak`として判定対象に含めるモデル名の正規表現パターンをリストで与え、
            マッチした全機体を対象に加える。また、"Any"を予約語とし、"Any"が与えられた場合は全機体を対象に加える。

            また、`modelNamesToBeExcludedForBreak`として判定対象から除外するモデル名の正規表現パターンをリストで与え、
            マッチした全機体を対象から除外する。

            両方にマッチした場合は除外を優先する。

            modelConfigの関係する項目は以下のとおり。
            - modelNamesToBeConsideredForBreak (list[str])
            - modelNamesToBeExcludedForBreak (list[str])

        ## 場外判定対象機

            戦域の外に存在している場合に場外ペナルティを与えるかどうか。
            現実装では全ての機体を判定対象とする。

    # 終了条件について

        終了条件はステップの終了時に以下のいずれかを満たしていた場合とする。

        (1) いずれかの陣営の全滅判定対象機が全滅（被撃墜または墜落）し、かつ残存陣営の以後の被撃墜の可能性が消滅したとき
        (2) いずれかの陣営の突破判定対象機が相手の防衛ラインを突破し、かつ双方の以後の撃墜の可能性が消滅したとき
        (3) 戦闘開始からminTimeが経過した以降において、両陣営の全機が各々の防衛ラインより後方まで下がったとき
        (4) 制限時間maxTimeが経過したとき
        (5) いずれかの陣営の総得点又は累積ペナルティがpDisq以下となったとき

        ただし、以下のフラグをmodelConfigで指定することにより終了条件を変更可能とする。

        (a) enableAdditionalTime (bool)・・・Falseとした場合、(1)及び(2)から「残存陣営の以後の被撃墜の可能性が消滅したとき」という条件を削除する。
        (b) terminalAtElimination (bool)・・・Falseとした場合、(1)の条件を削除する。
        (c) terminalAtBreak (bool)・・・Falseとした場合、(2)の条件を削除する。
        (d) independentPenalty (bool)・・・Trueとした場合、(5)でpDisqと比較する対象を総得点ではなく累積ペナルティ分のみとする。Falseとした場合、加点を含む総得点で判定する。

        なお、「以後の撃墜の可能性が消滅したとき」とは、「生存している陣営に対立する陣営の航空機、飛翔中誘導弾が全て消滅したとき」を指すものとする。

        また、ステップの終了時に同時に複数の終了条件が満たされていた場合は、以下の優先度で最も高いものによる終了とみなす。
        
        高　(1) > (2) > (5) > (3) > (4)　低

    # 勝敗の判定について

        modelConfigにおいて`independentPenalty`フラグをtrueとした場合であって、
        終了条件(5)により終了(失格)となった場合については、
        総得点又は累積ペナルティがpDisq以下となった陣営を敗北とし、両陣営が同時に失格となっていた場合は引き分けとする。

        それ以外の条件で終了した場合は、戦闘終了時の総得点が多い方を勝者とする。
        なお、`independentPenalty`フラグがtrueの場合、累積ペナルティ分は勝敗判定用の総得点には含まれない。

    # 得点計算について

        ## 撃墜

            全滅判定対象機が撃墜された場合、撃墜した側の陣営に`pHit`で指定されたFactoryモデル名に応じた得点を加算する。

            `pHit`は正規表現パターンをキーとしたdictであり、モデル名がマッチした全要素の値をその機体の撃墜点として加算する。

            また、`pHitPerAirCraft`がtrueの場合、`pHit`の各要素の値はそのまま1機あたりの得点を表す。
            falseの場合は1陣営分の総量として、戦闘開始時に各`pHit`のキーにマッチする機体の数で除した値を1機あたりの得点とする。

            modelConfigの関係する項目は以下のとおり。
            - pHit (dict[str, float])
            - pHitPerAircraft (bool)

        ## 墜落

            modelConfigにおいて`pCrashAsPenalty`フラグをfalseとした場合、
            全滅判定対象機が自ら墜落した場合、所属陣営に`pCrash`で指定されたFactoryモデル名に応じた得点を減算する。
            trueとした場合は得点ではなく後述のペナルティとして扱う。

            `pCrash`は正規表現パターンをキーとしたdictであり、モデル名がマッチした全要素の値をその機体の撃墜点として減算する。

            また、`pCrashPerAirCraft`がtrueの場合、`pCrash`の各要素の値はそのまま1機あたりの得点を表す。
            falseの場合は1陣営分の総量として、戦闘開始時に各`pCrash`のキーにマッチする機体の数で除した値を1機あたりの得点とする。

            さらに、戦闘終了時に帰還不可能と判定された全滅判定対象機についても同様にペナルティを課す。
            ただし、この戦闘終了時の追加ペナルティは終了条件(5)のpDisqによる失格判定には含めない。

            modelConfigの関係する項目は以下のとおり。
            - pCrash (dict[str, float])
            - pCrashPerAircraft (bool)

        ## 生存

            戦闘終了時に帰還可能な全滅判定対象機が生存していた場合、所属陣営に`pAlive`で指定されたFactoryモデル名に応じた得点を加算する。

            `pAlive`は正規表現パターンをキーとしたdictであり、モデル名がマッチした全要素の値をその機体の撃墜点として加算する。

            また、`pAlivePerAirCraft`がtrueの場合、`pAlive`の各要素の値はそのまま1機あたりの得点を表す。
            falseの場合は1陣営分の総量として、戦闘開始時に各`pAlive`のキーにマッチする機体の数で除した値を1機あたりの得点とする。

            modelConfigの関係する項目は以下のとおり。
            - pAlive (dict[str, float])
            - pAlivePerAircraft (bool)

        ## 防衛ラインの突破

            突破判定対象機が相手の防衛ラインを突破した場合、突破した側の陣営に`pBreak`点を加算する。

            また、終了条件(1)を満たしたとき、未突破の陣営に「そこから突破して更に帰還可能な」突破判定対象機が存在している場合も、`pBreak`点を加算する。
            (残った側が以後妨害を受けずに突破できるとみなせるため。)

        ## 進出度合いによる優勢度

            終了条件(4)又は(5)を満たして終了した若しくは両陣営が全滅したときに、いずれの陣営も未突破だったならば、
            両陣営の最前線にいた機体の直近の位置どうしを結んだ線分の中点のy座標の絶対値に応じ、より進出している陣営に1kmあたり`pAdv`点を加算する。

        ## ペナルティについて

            以下の要素については減点対象とし、一定以上累積した場合に失格とする。

            modelConfigにおいて`independentPenalty`フラグにより判定方法を変更する。
            
            - falseの場合: 前述の得点計算と合算し、勝敗判定に用いる総得点に含める。また、失格判定は総得点に対して行なう。
            - trueの場合: 前述の得点計算とは独立に計算し、勝敗判定に用いる総得点からは除外する。また、失格判定は累積ペナルティ分のみで行なう。

        ### 墜落

            modelConfigにおいて`pCrashAsPenalty`フラグをtrueとした場合、
            全滅判定対象機が自ら墜落した場合の減点をペナルティとして失格判定の対象に含める。
            ただし、`independentPenalty`フラグがfalseの場合は結局得点計算に合算されるため`pCrashAsPenalty`フラグの指定は意味を持たない。

        ### 場外

            場外判定対象機が戦域の外に出ていたとき、戦域端からの距離に応じて1kmあたりpOut点/秒を、毎回のonInnerStepで減算する。
            複数の機体が同時に戦域の外に出ていた場合は、１機ずつ個別に計算する。

            また、modelConfigにおいて`regardDefenseLineAsPenaltyLine`をtrueとした場合は東西南北全ての戦域外をペナルティの対象とし、
            falseとした場合は南北方向のみをペナルティの対象とする。
 */
ASRC_DECLARE_DERIVED_CLASS_WITH_TRAMPOLINE(BasicAACRuler01,asrc::core::Ruler)
    public:
    using PhysicalAsset=asrc::core::PhysicalAsset;
    enum class DownReason{
        CRASH,
        HIT
    };
    enum class EndReason{
        ELIMINATION,
        BREAK,
        TIMEUP,
        WITHDRAWAL,
        PENALTY,
        NOTYET
    };
    bool debug;
    double minTime;
    double dLine,dOut,hLim;
    std::string westSider,eastSider;
    double pDisq;//失格となる得点(陣営単位)
    double pBreak;//突破時の得点(陣営単位)
    std::map<std::string,std::vector<std::string>> modelNamesToBeConsideredForBreak;//!< 突破判定対象のFactoryモデルを陣営ごとに指定。正規表現で指定可能であり、いずれかにマッチすれば対象となる。また、"Any"を全モデルを対象とすることを表す予約語として扱う。
    std::map<std::string,std::vector<std::string>> modelNamesToBeExcludedForBreak;//!< 突破判定対象から除外するFactoryモデルを陣営ごとに指定。正規表現で指定可能であり、いずれかにマッチすれば除外される。
    std::map<std::string,double> pHit;//!< 被撃墜時の相手陣営への加点。Factoryモデルごとに指定する。正規表現で指定可能であり、マッチした全要素の得点が加算される。いずれにもマッチしない場合の得点として"Default"を予約語として扱う。"Default"を指定しなかった場合は0とする。なお、後述の全滅判定対象になっているモデルのみ加点対象となる。
    std::map<std::string,std::map<std::string,double>> pHitScale;//!< 陣営、Factoryモデルごとの各機の被撃墜時の相手陣営への加点の倍率。
    std::map<std::string,double> pCrash;//!< 墜落時の自陣営への減点。Factoryモデルごとに指定する。正規表現で指定可能であり、マッチした全要素の得点が加算される。いずれにもマッチしない場合の得点として"Default"を予約語として扱う。"Default"を指定しなかった場合は0とする。なお、後述の全滅判定対象になっているモデルのみ減点対象となる。
    std::map<std::string,std::map<std::string,double>> pCrashScale;//!< 陣営、Factoryモデルごとの各機の墜落時の自陣営への減点の倍率。
    std::map<std::string,double> pAlive;//!< 終了時の自陣営への加点(生存点)。Factoryモデルごとに指定する。正規表現で指定可能であり、マッチした全要素の得点が加算される。いずれにもマッチしない場合の得点として"Default"を予約語として扱う。"Default"を指定しなかった場合は0とする。なお、後述の全滅判定対象になっているモデルのみ加点対象となる。
    std::map<std::string,std::map<std::string,double>> pAliveScale;//!< 陣営、Factoryモデルごとの各機の墜落時の自陣営への減点の倍率。
    std::map<std::string,std::vector<std::string>> modelNamesToBeConsideredForElimination;//!< 全滅判定対象のFactoryモデルを陣営ごとに指定。正規表現で指定可能であり、いずれかにマッチすれば対象となる。"Any"を全モデルを対象とすることを表す予約語として扱う。
    std::map<std::string,std::vector<std::string>> modelNamesToBeExcludedForElimination;//!< 全滅判定対象から除外するFactoryモデルを陣営ごとに指定。正規表現で指定可能であり、いずれかにマッチすれば除外される。
    double pAdv;//優勢度による得点(陣営単位)
    double pOut;//場外のペナルティ(Factoryモデルに依存せず、機体単位で計算)
    std::map<std::string,std::map<std::string,int>> crashCount,hitCount;//[team,[model,count]]
    std::map<std::string,double> leadRange;
    std::map<std::string,double> lastDownPosition;
    std::map<std::string,DownReason> lastDownReason;
    std::map<std::string,double> outDist;
    std::map<std::string,double> eliminatedTime;
    std::map<std::string,double> breakTime;
    std::map<std::string,double> disqTime;
    std::map<std::string,Eigen::Vector2d> forwardAx,sideAx;
    EndReason endReason,endReasonSub;
    bool regardDefenseLineAsPenaltyLine; //!< 東西の防衛ラインを場外ペナルティラインとしても扱うかどうか。
    bool pCrashAsPenalty;//!< 墜落をペナルティとして扱うかどうか。
    bool pHitPerAircraft; //!< configで指定する撃墜点を、1機あたりの点数とするか1陣営中の該当機種全体での合計点数とするかのフラグ。
    bool pCrashPerAircraft; //!< configで指定する墜落点を、1機あたりの点数とするか1陣営中の該当機種全体での合計点数とするかのフラグ。
    bool pAlivePerAircraft; //!< configで指定する生存点を、1機あたりの点数とするか1陣営中の該当機種全体での合計点数とするかのフラグ。
    bool enableAdditionalTime;//相討ちの有無を確認するために、一方が全滅又は突破後の計算を継続するかどうか。
    bool terminalAtElimination;//終了条件(1)を有効とするかどうか
    bool terminalAtBreak;//終了条件(2)を有効とするかどうか
    bool considerFuelConsumption;//燃料消費を考慮するかどうか。
    double fuelMargin;//帰還可否判定時の燃料消費量のマージン。最適巡航時の(1+fuelMargin)倍の燃料を消費するものとする。
    std::map<std::string,double> distanceFromBase;//各陣営の防衛ラインから基地までの距離。帰還に必要な燃料を計算するために指定する。また、進出に要する燃料を消費した状態で開始する。
    std::vector<std::string> deadFighters;//撃墜or墜落済のFighterのfullNameのリスト。得点の二重計算防止用。
    //constructors & destructor
    using BaseType::BaseType;
    //functions
    virtual void initialize() override;
    virtual void serializeInternalState(asrc::core::util::AvailableArchiveTypes & archive, bool full) override;
    void debugPrint(const std::string& reason,const std::string& team,double value);
    virtual void onCrash(const nl::json& args);
    virtual void onHit(const nl::json& args);
    virtual void onEpisodeBegin() override;
    virtual void onValidationEnd() override;
    virtual void onInnerStepBegin() override;
    virtual void onInnerStepEnd() override;
    virtual void checkDone() override;
    virtual std::string getEndReason() const override;
    virtual std::string getEndReasonSub() const;
    virtual void setEndReason(const std::string& newReason) override;
    virtual void setEndReasonSub(const std::string& newReason);
    virtual void _setupPDownConfig(
        std::map<std::string,double>& _config,
        const nl::json& _modelConfig,
        const std::string& _key,
        double _defaultValue);
    virtual void _setupPDownScale(
        std::map<std::string,std::map<std::string,double>>& _scale,
        const std::map<std::string,double>& _config,
        bool _perAircraft);
    virtual double _getPDownImpl(
        const std::map<std::string,std::map<std::string,double>>& _scale,
        const std::map<std::string,double>& _config,
        const std::string& _team,
        const std::string& _modelName) const;
    virtual double getPHit(const std::string& team,const std::string& modelName) const;
    virtual double getPCrash(const std::string& team,const std::string& modelName) const;
    virtual double getPAlive(const std::string& team,const std::string& modelName) const;
    virtual int& getCrashCount(const std::string& team,const std::string& modelName);
    virtual int& getHitCount(const std::string& team,const std::string& modelName);
    virtual bool isToBeConsideredForBreak(const std::string& team,const std::string& modelName);
    virtual bool isToBeConsideredForElimination(const std::string& team,const std::string& modelName);
    virtual bool checkHitPossibility() const;
    virtual bool isReturnableToBase(const std::shared_ptr<PhysicalAsset>& asset) const;
    virtual bool isBreakableAndReturnableToBase(const std::shared_ptr<PhysicalAsset>& asset) const;
};
DEFINE_SERIALIZE_ENUM_AS_STRING(BasicAACRuler01::DownReason)
DEFINE_SERIALIZE_ENUM_AS_STRING(BasicAACRuler01::EndReason)
ASRC_DECLARE_DERIVED_TRAMPOLINE(BasicAACRuler01)
    virtual std::string getEndReasonSub() const override{
        PYBIND11_OVERRIDE(std::string,Base,getEndReasonSub);
    }
    virtual void setEndReasonSub(const std::string& newReason) override{
        PYBIND11_OVERRIDE(void,Base,setEndReasonSub,newReason);
    }
    virtual void onCrash(const nl::json& args) override{
        PYBIND11_OVERRIDE(void,Base,onCrash,args);
    }
    virtual void onHit(const nl::json& args) override{
        PYBIND11_OVERRIDE(void,Base,onHit,args);
    }
    virtual void _setupPDownConfig(
        std::map<std::string,double>& _config,
        const nl::json& _modelConfig,
        const std::string& _key,
        double _defaultValue) override{
        PYBIND11_OVERRIDE(void,Base,_setupPDownConfig,_config,_modelConfig,_key,_defaultValue);
    }
    virtual void _setupPDownScale(
        std::map<std::string,std::map<std::string,double>>& _scale,
        const std::map<std::string,double>& _config,
        bool _perAircraft) override{
        PYBIND11_OVERRIDE(void,Base,_setupPDownScale,_scale,_config,_perAircraft);
    }
    virtual double _getPDownImpl(
        const std::map<std::string,std::map<std::string,double>>& _scale,
        const std::map<std::string,double>& _config,
        const std::string& _team,
        const std::string& _modelName) const override{
        PYBIND11_OVERRIDE(double,Base,_getPDownImpl,_scale,_config,_team,_modelName);
    }
};
void exportBasicAACRuler01(py::module &m, const std::shared_ptr<asrc::core::FactoryHelper>& factoryHelper);

ASRC_PLUGIN_NAMESPACE_END

ASRC_PYBIND11_MAKE_OPAQUE(std::map<std::string,ASRC_PLUGIN_NAMESPACE::BasicAACRuler01::DownReason>);
