#include "AliGenParam.h"
#include "AliGenMUONlib.h"
#include "AliRun.h"
#include "AliPythia.h"
#include <TDirectory.h>
#include <TFile.h>
#include <TTree.h>
#include <stdlib.h>
#include <TParticle.h>

ClassImp(AliGenParam)

//------------------------------------------------------------

  //Begin_Html
  /*
    <img src="picts/AliGenParam.gif">
  */
  //End_Html

//____________________________________________________________
//____________________________________________________________
AliGenParam::AliGenParam()
                 :AliGenerator()
{
  fPtPara = 0;
  fYPara  = 0;
  fParam  = jpsi_p;
  fAnalog = analog;
  SetCutOnChild();
}

//____________________________________________________________
AliGenParam::AliGenParam(Int_t npart, Param_t param) 
//				   Double_t (*PtPara)(Double_t*, Double_t*), 
//				   Double_t (*YPara) (Double_t* ,Double_t*))
    :AliGenerator(npart)
{
  //
  //  fName="HMESONpara";
  //  fTitle="Heavy Mesons Parametrisation";
  fPtParaFunc = AliGenMUONlib::GetPt(param);
  fYParaFunc  = AliGenMUONlib::GetY(param);
  fIpParaFunc = AliGenMUONlib::GetIp(param);
  
  fPtPara = 0;
  fYPara  = 0;
  fParam  = param;
  fAnalog = analog;
  fChildSelect.Set(5);
  for (Int_t i=0; i<5; i++) fChildSelect[i]=0;
  ForceDecay();
  SetCutOnChild();
}

//____________________________________________________________
AliGenParam::~AliGenParam()
{
    delete  fPtPara;
    delete  fYPara;
}

//____________________________________________________________
void AliGenParam::Init()
{
    SetMC(new AliPythia());
    fPythia= (AliPythia*) fgMCEvGen;

//  End of the test !!!
  //Begin_Html
  /*
    <img src="picts/AliGenParam.gif">
  */
  //End_Html
 
  fPtPara = new TF1("Pt-Parametrization",fPtParaFunc,fPtMin,fPtMax,0);
  fYPara  = new TF1("Y -Parametrization",fYParaFunc,fYMin,fYMax,0);
  TF1* PtPara = new TF1("Pt-Parametrization",fPtParaFunc,0,15,0);
  TF1* YPara  = new TF1("Y -Parametrization",fYParaFunc,-6,6,0);

//
// dN/dy| y=0
  Double_t y1=0;
  Double_t y2=0;
  
  fdNdy0=fYParaFunc(&y1,&y2);
//
// Integral over generation region
  Float_t IntYS  = YPara ->Integral(fYMin, fYMax);
  Float_t IntPt0 = PtPara->Integral(0,15);
  Float_t IntPtS = PtPara->Integral(fPtMin,fPtMax);
  Float_t PhiWgt=(fPhiMax-fPhiMin)/2./TMath::Pi();
  if (fAnalog) {
     fYWgt  = IntYS/fdNdy0;
     fPtWgt = IntPtS/IntPt0;
     fParentWeight = fYWgt*fPtWgt*PhiWgt/fNpart;
  } else {
      fYWgt = IntYS/fdNdy0;
      fPtWgt = (fPtMax-fPtMin)/IntPt0;
      fParentWeight = fYWgt*fPtWgt*PhiWgt/fNpart;
  }
//
// particle decay related initialization
  fPythia->DefineParticles();
// semimuonic decays of charm and beauty
  fPythia->ForceDecay(fForceDecay);
//
    switch (fForceDecay) 
    {
    case semielectronic:
    case dielectron:
    case b_jpsi_dielectron:
    case b_psip_dielectron:
	fChildSelect[0]=11;	
	break;
    case semimuonic:
    case dimuon:
    case b_jpsi_dimuon:
    case b_psip_dimuon:
	fChildSelect[0]=13;
	break;
    case pitomu:
	fChildSelect[0]=13;
	break;
    case katomu:
	fChildSelect[0]=13;
	break;
    }

}

//____________________________________________________________
void AliGenParam::Generate()
{
// Generate 'npart' of heavy mesons (J/Psi, upsilon or phi) in the
// the desired theta, phi and momentum windows; Gaussian smearing 
// on the vertex is done if selected


  //printf("Generate !!!!!!!!!!!!!\n");

  Float_t polar[3]= {0,0,0};
  //
  Float_t origin[3], origin0[3];
  Float_t pt, pl, ptot;
  Float_t phi, theta;
  Float_t p[3], pc[3], och[3], pch[10][3];
  Float_t ty, xmt;
  Int_t nt, i, j, kfch[10];
  Float_t  wgtp, wgtch;
  Double_t dummy;
  static TClonesArray *particles;
  //
  if(!particles) particles=new TClonesArray("TParticle",1000);
  //
  Float_t random[6];
  for (j=0;j<3;j++) origin0[j]=fOrigin[j];
  if(fVertexSmear==perEvent) {
      gMC->Rndm(random,6);
      for (j=0;j<3;j++) {
	  origin0[j]+=fOsigma[j]*TMath::Cos(2*random[2*j]*TMath::Pi())*
	      TMath::Sqrt(-2*TMath::Log(random[2*j+1]));
      }
  }
  Int_t ipa=0;
  while (ipa<fNpart) {
    while(1) {
//
// particle type
	  Int_t Ipart = fIpParaFunc();
	  fChildWeight=(fPythia->GetBraPart(Ipart))*fParentWeight;	  
	  Float_t am=fPythia->GetPMAS(fPythia->LuComp(Ipart),1);
	  gMC->Rndm(random,2);
//
// phi
	  phi=fPhiMin+random[0]*(fPhiMax-fPhiMin);
//
// y
	  ty=Float_t(TMath::TanH(fYPara->GetRandom()));
//
// pT
	  if (fAnalog) {
	      pt=fPtPara->GetRandom();
	      wgtp=fParentWeight;
	      wgtch=fChildWeight;
	  } else {
	      pt=fPtMin+random[1]*(fPtMax-fPtMin);
	      Double_t ptd=pt;
	      wgtp=fParentWeight*fPtParaFunc(& ptd, &dummy);
	      wgtch=fChildWeight*fPtParaFunc(& ptd, &dummy);
	  }
	  xmt=sqrt(pt*pt+am*am);
	  pl=xmt*ty/sqrt(1.-ty*ty);
	  theta=TMath::ATan2(pt,pl);
	  if(theta<fThetaMin || theta>fThetaMax) continue;
	  ptot=TMath::Sqrt(pt*pt+pl*pl);
	  if(ptot<fPMin || ptot>fPMax) continue;
	  p[0]=pt*TMath::Cos(phi);
	  p[1]=pt*TMath::Sin(phi);
	  p[2]=pl;
	  if(fVertexSmear==perTrack) {
	      gMC->Rndm(random,6);
	      for (j=0;j<3;j++) {
		  origin0[j]=
		      fOrigin[j]+fOsigma[j]*TMath::Cos(2*random[2*j]*TMath::Pi())*
		      TMath::Sqrt(-2*TMath::Log(random[2*j+1]));
	      }
	  }
//
// use lujet to decay particle

	  Float_t energy=TMath::Sqrt(ptot*ptot+am*am);
	  fPythia->DecayParticle(Ipart,energy,theta,phi);
	  //	  fPythia->LuList(1);


	  //printf("origin0 %f %f %f\n",origin0[0],origin0[1],origin0[2]);
	  //printf("fCutOnChild %d \n",fCutOnChild);
//
// select muons
	  Int_t np=fPythia->ImportParticles(particles,"All");
          //printf("np     %d \n",np);
	  Int_t ncsel=0;
	  for (i = 1; i<np; i++) {
	      TParticle *  iparticle = (TParticle *) particles->At(i);
	      Int_t kf = iparticle->GetPdgCode();
              //printf("kf %d\n",kf);
//
// children
	      if (ChildSelected(TMath::Abs(kf)))
	      {
		  pc[0]=iparticle->Px();
		  pc[1]=iparticle->Py();
		  pc[2]=iparticle->Pz();
		  och[0]=origin0[0]+iparticle->Vx()/10;
		  och[1]=origin0[1]+iparticle->Vy()/10;
		  och[2]=origin0[2]+iparticle->Vz()/10;
		  if (fCutOnChild) {
		    Float_t PtChild=TMath::Sqrt(pc[0]*pc[0]+pc[1]*pc[1]);
		    Float_t PChild=TMath::Sqrt(PtChild*PtChild+pc[2]*pc[2]);
		    Float_t ThetaChild=TMath::ATan2(PtChild,pc[2]);
		    Float_t PhiChild=TMath::ATan2(pc[1],pc[0])+TMath::Pi();
		    if ((PtChild   > fPtMin   && PtChild   <fPtMax)      &&
			(PChild    > fPMin    && PChild    <fPMax)       &&
			(ThetaChild>fThetaMin && ThetaChild<fThetaMax)   &&
			(PhiChild  >  fPhiMin && PhiChild  <fPhiMax))
		      {
			pch[ncsel][0]=pc[0];
			pch[ncsel][1]=pc[1];
			pch[ncsel][2]=pc[2];
			kfch[ncsel]=kf;
			ncsel++;
		      } else {
			ncsel=-1;
			break;
		      } // child kine cuts
		  } else {
		    pch[ncsel][0]=pc[0];
		    pch[ncsel][1]=pc[1];
		    pch[ncsel][2]=pc[2];
		    kfch[ncsel]=kf;
		    ncsel++;
		  } // if child selection
	      } // select muon
	  } // decay particle loop
	  Int_t iparent;
	  if ((fCutOnChild && ncsel >0) || !fCutOnChild){
	      ipa++;
//
// parent
	      gAlice->
		  SetTrack(0,-1,Ipart,p,origin,polar,0,"Primary",nt,wgtp);
	      iparent=nt;

	      for (i=0; i< ncsel; i++) {
		  gAlice->SetTrack(fTrackIt,iparent,kfch[i],
				   &pch[i][0],och,polar,
				   0,"Decay",nt,wgtch);
		  gAlice->KeepTrack(nt); 
	      }
	      
	  } // kinematic selection
	  break;
    } // while
  } // event loop
}

Bool_t AliGenParam::ChildSelected(Int_t ip)
{
    for (Int_t i=0; i<5; i++)
    {
	if (fChildSelect[i]==ip) return kTRUE;
    }
    return kFALSE;
}

Bool_t AliGenParam::KinematicSelection(TParticle *particle)
{
    Float_t px=particle->Px();
    Float_t py=particle->Py();
    Float_t pz=particle->Pz();
//
// momentum cut
    Float_t p=TMath::Sqrt(px*px+py*py+pz*pz);
    if (p > fPMax || p < fPMin) 
    {
//	printf("\n failed p cut %f %f %f \n",p,fPMin,fPMax);
	return kFALSE;
    }
    Float_t pt=TMath::Sqrt(px*px+py*py);
    
//
// theta cut
    Float_t  theta = Float_t(TMath::ATan2(Double_t(pt),Double_t(p)));
    if (theta > fThetaMax || theta < fThetaMin) 
    {
//	printf("\n failed theta cut %f %f %f \n",theta,fThetaMin,fThetaMax);
	return kFALSE;
    }

    return kTRUE;
}




