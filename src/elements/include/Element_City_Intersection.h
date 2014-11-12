/*                                              -*- mode:C++ -*-
  Element_City_Intersection.h Street hub for city simulation
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Element_City_Intersection.h Street hub for city simulation
  \author Trent R. Small.
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_CITY_INTERSECTION_H
#define ELEMENT_CITY_INTERSECTION_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "Dirs.h"
#include "itype.h"
#include "P3Atom.h"

namespace MFM
{

#define CITY_VERSION 1

  template<class CC>
  class Element_City_Intersection : public Element<CC>
  {
    // Extract short names for parameter types
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum
    {
      R = P::EVENT_WINDOW_RADIUS,
      BITS = P::BITS_PER_ATOM,


      INITIALIZED_POS = P3Atom<P>::ATOM_FIRST_STATE_BIT,
      INITIALIZED_LEN = 1,

      CANAL_MAP1_POS = INITIALIZED_POS + INITIALIZED_LEN,
      CANAL_MAP1_LEN = CityConstants::CITY_BUILDING_COUNT,

      CANAL_MAP2_POS = CANAL_MAP1_POS + CANAL_MAP1_LEN,
      CANAL_MAP2_LEN = CityConstants::CITY_BUILDING_COUNT
    };

    typedef BitField<BitVector<BITS>, VD::U32, INITIALIZED_LEN, INITIALIZED_POS> AFInitBits;

    typedef BitField<BitVector<BITS>, VD::U32, CANAL_MAP1_LEN, CANAL_MAP1_POS> AFCanal1;
    typedef BitField<BitVector<BITS>, VD::U32, CANAL_MAP2_LEN, CANAL_MAP2_POS> AFCanal2;

   private:
    ElementParameterS32<CC> m_minCreatedStreets;

    void RandomizeCanal(T& us, Random& rand) const
    {
      for(u32 i = 0; i < CANAL_MAP1_LEN; i++)
      {
        AFCanal1::Write(this->GetBits(us), !!rand.CreateBool());
        AFCanal2::Write(this->GetBits(us), !!rand.CreateBool());
      }
    }

    Dir GetCanalDir(const T& us, u32 buildingType) const
    {
      u32 ans = 0;
      u32 top = AFCanal1::Read(this->GetBits(us));
      u32 bot = AFCanal2::Read(this->GetBits(us));

      ans = ((top & (1 << buildingType)) > 0);
      ans <<= 1;

      ans |= ((bot & (1 << buildingType)) > 0);

      if(ans < 0 || ans > 3)
      {
        FAIL(ILLEGAL_STATE);
      }

      return (Dir)(ans * 2);
    }

    void SetCanalDir(T& us, u32 destType, Dir dir) const
    {
      if(Dirs::IsCorner(dir))
      {
        FAIL(ILLEGAL_ARGUMENT);
      }
      dir /= 2;
      if(dir > 3)
      {
        FAIL(ILLEGAL_ARGUMENT);
      }

      u32 topBit = (dir & 2) >> 1;
      u32 botBit = (dir & 1);

      u32 topWord = AFCanal1::Read(this->GetBits(us));
      u32 botWord = AFCanal2::Read(this->GetBits(us));

      if(topBit)
      {
        topWord |= (1 << destType);
      }
      else
      {
        topWord &= ~(1 << destType);
      }

      if(botBit)
      {
        botWord |= (1 << destType);
      }
      else
      {
        botWord &= ~(1 << destType);
      }

      AFCanal1::Write(this->GetBits(us), topWord);
      AFCanal2::Write(this->GetBits(us), botWord);
    }

   public:
    static Element_City_Intersection THE_INSTANCE;

    static const u32 TYPE()
    {
      return THE_INSTANCE.GetType();
    }

    bool IsInitialized(const T& us) const
    {
      return AFInitBits::Read(this->GetBits(us)) > 0;
    }

    void Uninitialize(T& us) const
    {
      AFInitBits::Write(this->GetBits(us), 0);
    }

    void SetInitialized(T& us) const
    {
      AFInitBits::Write(this->GetBits(us), 1);
    }

    Element_City_Intersection() :
      Element<CC>(MFM_UUID_FOR("CityIntersection", CITY_VERSION)),
      m_minCreatedStreets(this, "minCreatedStreets",
                          "Minimum streets created",
                          "Minimum streets created", 1, 4, 4)
    {
      Element<CC>::SetAtomicSymbol("In");
      Element<CC>::SetName("City Intersection");
    }

    virtual const T& GetDefaultAtom() const
    {

      static T defaultAtom(TYPE(), 0, 0, 0);
      Uninitialize(defaultAtom);

      return defaultAtom;

    }

    virtual u32 PercentMovable(const T& you, const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xff303030;
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff181818;
    }

    virtual const char* GetDescription() const
    {
      return "Basic element for city simulation.";
    }

    virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    {
      return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    }

   private:

    void FillIfNotType(EventWindow<CC>& window, SPoint& offset, u32 type, const T& atom) const
    {
      if(window.GetRelativeAtom(offset).GetType() != type)
      {
        window.SetRelativeAtom(offset, atom);
      }
    }

   private:
    void InitializeIntersection(T& atom, EventWindow<CC>& window) const;

    void UTurnCar(EventWindow<CC>& window, SPoint& carAt) const;

    void DoRouting(EventWindow<CC>& window) const;

    u32 GetStreetType() const;

    u32 GetCarType() const;

    u32 GetSidewalkType() const;

    u32 IsRelDirCarOrStreet(EventWindow<CC> window, Dir d) const;

    bool CanalIsValid(EventWindow<CC>& window, u32 destType) const;

    Dir FindBestRouteCanal(EventWindow<CC>& window, u32 destinationType,
                           Dir comingFrom) const;

    Dir FindBestRouteStandard(EventWindow<CC>& window, u32 destinationType,
                              Dir comingFrom) const;

    Dir FindRandomRoute(EventWindow<CC>& window) const ;

    void CreateStreetFromEmpty(EventWindow<CC>& window, Dir d) const;

    void CreateSidewalkFromEmpty(EventWindow<CC>& window, Dir d) const;

   public:
    virtual void Behavior(EventWindow<CC>& window) const
    {
      if(!IsInitialized(window.GetCenterAtom()))
      {
        T newAtom = window.GetCenterAtom();
        InitializeIntersection(newAtom, window);
        RandomizeCanal(newAtom, window.GetRandom());
        SetInitialized(newAtom);

        window.SetCenterAtom(newAtom);
      }
      else
      {
        DoRouting(window);
      }
    }
  };

  template <class CC>
  Element_City_Intersection<CC> Element_City_Intersection<CC>::THE_INSTANCE;
}

#include "Element_City_Intersection.tcc"

#endif /* ELEMENT_CITY_INTERSECTION_H */
