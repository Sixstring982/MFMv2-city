#include "main.h"

namespace MFM
{
  /////
  // Standard model

  typedef ParamConfig<96,4,8,40> OurParamConfigStd;
  typedef P3Atom<OurParamConfigStd> OurAtomStd;
  typedef CoreConfig<OurAtomStd, OurParamConfigStd> OurCoreConfigStd;
  typedef GridConfig<OurCoreConfigStd, 5, 3> OurGridConfigStd;

  /////
  // Tiny model

  typedef ParamConfig<96,4,8,32> OurParamConfigTiny;
  typedef P3Atom<OurParamConfigTiny> OurAtomTiny;
  typedef CoreConfig<OurAtomTiny, OurParamConfigTiny> OurCoreConfigTiny;
  typedef GridConfig<OurCoreConfigTiny, 2, 2> OurGridConfigTiny;

  /////
  // Larger model

  typedef ParamConfig<96,4,8,48> OurParamConfigBig;
  typedef P3Atom<OurParamConfigBig> OurAtomBig;
  typedef CoreConfig<OurAtomBig, OurParamConfigBig> OurCoreConfigBig;
  typedef GridConfig<OurCoreConfigBig, 8, 5> OurGridConfigBig;

  template <class GC>
  struct MFMCDriver : public AbstractDualDriver<GC>
  {
  private:

    typedef AbstractDualDriver<GC> Super;
    typedef typename Super::CC CC;

    virtual void DefineNeededElements()
    {
      this->NeedElement(&Element_Empty<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Wall<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Res<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Dreg<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Sorter<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Data<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Emitter<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Consumer<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Block<CC>::THE_INSTANCE);
      this->NeedElement(&Element_ForkBomb1<CC>::THE_INSTANCE);
      this->NeedElement(&Element_ForkBomb2<CC>::THE_INSTANCE);
      this->NeedElement(&Element_ForkBomb3<CC>::THE_INSTANCE);
      this->NeedElement(&Element_AntiForkBomb<CC>::THE_INSTANCE);
      this->NeedElement(&Element_MQBar<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Mover<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Indexed<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Fish<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Shark<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_Sq1<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_L12<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_R12<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Xtal_General<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Creg<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Dmover<CC>::THE_INSTANCE);
      this->NeedElement(&Element_CheckerForkBlue<CC>::THE_INSTANCE);
      this->NeedElement(&Element_CheckerForkRed<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Wanderer_Cyan<CC>::THE_INSTANCE);
      this->NeedElement(&Element_Wanderer_Magenta<CC>::THE_INSTANCE);
      this->NeedElement(&Element_City_Street<CC>::THE_INSTANCE);
      this->NeedElement(&Element_City_Sidewalk<CC>::THE_INSTANCE);
      this->NeedElement(&Element_City_Intersection<CC>::THE_INSTANCE);
      this->NeedElement(&Element_City_Park<CC>::THE_INSTANCE);
      this->NeedElement(&Element_City_Building<CC>::THE_INSTANCE);
      this->NeedElement(&Element_City_Car<CC>::THE_INSTANCE);
    }

  public:
    virtual void AddDriverArguments()
    {
      Super::AddDriverArguments();
    }

    virtual void OnceOnly(VArguments& args)
    {
      Super::OnceOnly(args);
    }

    virtual void ReinitEden()
    { }
  };

  static int RunSmall(int argc, const char** argv)
  {
    MFMCDriver<OurGridConfigTiny> sim;
    sim.Init(argc, argv);
    sim.Reinit();
    sim.Run();
    return 0;
  }

  static int RunMedium(int argc, const char** argv)
  {
    MFMCDriver<OurGridConfigStd> sim;
    sim.Init(argc, argv);
    sim.Reinit();
    sim.Run();
    return 0;
  }

  static int RunBig(int argc, const char** argv)
  {
    MFMCDriver<OurGridConfigBig> sim;
    sim.Init(argc, argv);
    sim.Reinit();
    sim.Run();
    return 0;
  }
}

static bool EndsWith(const char *string, const char* suffix)
{
  MFM::u32 slen = strlen(string);
  MFM::u32 xlen = strlen(suffix);
  return xlen <= slen && !strcmp(suffix, &string[slen - xlen]);
}

int main(int argc, const char** argv)
{
  MFM::DateTimeStamp stamper;
  MFM::LOG.SetTimeStamper(&stamper);
  MFM::LOG.SetByteSink(MFM::STDERR);
  MFM::LOG.SetLevel(MFM::LOG.MESSAGE);

  if (EndsWith(argv[0],"_s"))
  {
    return MFM::RunSmall(argc, argv);
  }

  if (EndsWith(argv[0],"_l"))
  {
    return MFM::RunBig(argc, argv);
  }

  return MFM::RunMedium(argc, argv);
}
