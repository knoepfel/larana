////////////////////////////////////////////////////////////////////////
// Class:       CosmicTagger
// Module Type: producer
// File:        CosmicTrackTagger_module.cc
//
// Generated at Mon Sep 24 18:21:00 2012 by Sarah Lockwitz using artmod
// from art v1_02_02.
// artmod -e beginJob -e reconfigure -e endJob producer trkf::CosmicTrackTagger
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iterator>

#include "Geometry/Geometry.h"
#include "Geometry/geo.h"

#include "RecoBase/SpacePoint.h"
#include "RecoBase/Hit.h"
#include "RecoBase/SpacePoint.h"
#include "RecoBase/Cluster.h"
#include "RecoBase/Shower.h"
#include "RecoBase/Track.h"

#include "AnalysisBase/CosmicTag.h"



#include "RecoAlg/SpacePointAlg.h"
#include "Utilities/AssociationUtil.h"
#include "Utilities/DetectorProperties.h"
#include "Utilities/LArProperties.h"

#include "TMatrixD.h"
#include "TDecompSVD.h"
#include "TVector3.h"
#include "TTree.h"
#include "TH1.h"
#include "TStopwatch.h"



class TTree;
class TH1;


namespace cosmic {
  class CosmicTrackTagger;
  class SpacePoint;
  class Track;
}



class cosmic::CosmicTrackTagger : public art::EDProducer {
public:
  explicit CosmicTrackTagger(fhicl::ParameterSet const & p);
  virtual ~CosmicTrackTagger();

  void produce(art::Event & e) override;

  void beginJob() override;
  void reconfigure(fhicl::ParameterSet const & p) override;
  void endJob() override;





private:


  //  float fTotalBoundaryLimit; // 15
  //  float f3DSpillDistance;    // 12
  //  int   fSpillVetoCtr;       // 2
  ////  int   fdTLimit;            // 8
  //int   fdWLimit;            // 8
  std::string fTrackModuleLabel;
  //  std::string fTrackAssocToClusterModuleLabel;
  //  std::string fClusterModuleLabel;
  //  int fDoTrackCheck;
  //  int fDoClusterCheck;
  //  int fClusterAssociatedToTracks;
  int fDetectorWidthTicks;
  float fTPCXBoundary, fTPCYBoundary, fTPCZBoundary;

  float fDetHalfHeight, fDetWidth, fDetLength;


};



cosmic::CosmicTrackTagger::CosmicTrackTagger(fhicl::ParameterSet const & p)
// :
// Initialize member data here.
{

  this->reconfigure(p);

  // Call appropriate Produces<>() functions here.

  produces< std::vector<anab::CosmicTag> >();
  produces< art::Assns<anab::CosmicTag, recob::Track> >();



}

cosmic::CosmicTrackTagger::~CosmicTrackTagger() {
  // Clean up dynamic memory and other resources here.
}

void cosmic::CosmicTrackTagger::produce(art::Event & e) {
  // Implementation of required member function here.

  std::unique_ptr< std::vector< anab::CosmicTag > > cosmicTagTrackVector( new std::vector<anab::CosmicTag> );
  std::unique_ptr< art::Assns<recob::Track, anab::CosmicTag > >    assnOutCosmicTagTrack( new art::Assns<recob::Track, anab::CosmicTag>);


  TStopwatch ts;


  art::Handle<std::vector<recob::Track> > Trk_h;
  e.getByLabel( fTrackModuleLabel, Trk_h );
  std::vector<art::Ptr<recob::Track> > TrkVec;
  art::fill_ptr_vector(TrkVec, Trk_h);



  /////////////////////////////////
  // LOOPING OVER INSPILL TRACKS
  /////////////////////////////////
  

    art::FindManyP<recob::Hit>        hitsSpill   (Trk_h, e, fTrackModuleLabel);
    //    art::FindManyP<recob::Cluster>    ClusterSpill(Trk_h, e, fTrackModuleLabel);

    for( unsigned int iTrack=0; iTrack<Trk_h->size(); iTrack++ ) {

      int isCosmic    =  0;

      art::Ptr<recob::Track>              tTrack  = TrkVec.at(iTrack);
      std::vector<art::Ptr<recob::Hit> >  HitVec  = hitsSpill.at(iTrack);

    

      // A BETTER WAY OF FINDING END POINTS:
      TVector3 tVector1 = tTrack->Vertex();
      TVector3 tVector2 = tTrack->End();


      float trackEndPt1_X = tVector1[0]; 
      float trackEndPt1_Y = tVector1[1]; 
      float trackEndPt1_Z = tVector1[2]; 
      float trackEndPt2_X = tVector2[0]; 
      float trackEndPt2_Y = tVector2[1]; 
      float trackEndPt2_Z = tVector2[2]; 

      if(trackEndPt1_X != trackEndPt1_X ) {
	std::cerr << "!!! FOUND A PROBLEM... the length is: " << tTrack->Length() << 
	  " np: " << tTrack->NumberTrajectoryPoints() << " id: " << tTrack->ID() << " " << tTrack << std::endl;
	for( size_t hh=0; hh<tTrack->NumberTrajectoryPoints(); hh++) {
	  std::cerr << hh << " " << tTrack->LocationAtPoint(hh)[0] << ", " <<
	    tTrack->LocationAtPoint(hh)[1] << ", " <<
	    tTrack->LocationAtPoint(hh)[2] << std::endl;
	}
	continue; // I don't want to deal with these "tracks"
      }





      //////////////////////////////////////////////////////////////////////
      //Let's check Cluster connections to pre & post spill planes first

//      // THIS REQUIRES THAT WE CHECK THE CLUSTERS WITHIN THIS TRACK LOOP
//      // SO DO SOMETHING LIKE:
//      std::vector <double> t1Times, t2Times;
//      std::vector <int> fail;
//      if(fDoClusterCheck) doTrackClusterCheck( ClusterVect, t1Times, t2Times, fail );
//
//
//      if( count( fail.begin(), fail.end(), 1 ) > 0 ) {
//	isCosmic = 4;
//      }



      /////////////////////////////////////
      // Getting first and last ticks
      /////////////////////////////////////
      float tick1 =  9999;
      float tick2 = -9999;
      
      for ( unsigned int p = 0; p < HitVec.size(); p++) {
	if( HitVec[p]->StartTime() < tick1 ) tick1 =  HitVec[p]->StartTime();
	if( HitVec[p]->StartTime() > tick2 ) tick2 =  HitVec[p]->StartTime();
      }
      

      /////////////////////////////////////////////////////////
      // Are any of the ticks outside of the ReadOutWindow ?
      /////////////////////////////////////////////////////////
      if(tick1 < fDetectorWidthTicks || tick2 > 2*fDetectorWidthTicks ) {
	isCosmic = 1;
      }
      // if(isCosmic == 4) {
      // 	std::cerr << "I don't know what's going on here. The ticks are " << tick1 << " " << tick2 << std::endl;
      // 	//	for( unsigned int mm=0;mm<t1Times.size(); mm++ ) std::cerr << t1Times.at(mm) << ", "<< t2Times.at(mm) <<std::endl;
      // }


    
      /////////////////////////////////
      // Now check Y & Z boundaries:
      /////////////////////////////////
      int nBd = 0;
      if(isCosmic==0 ) {
	if(fabs(trackEndPt1_Y - fDetHalfHeight) < fTPCYBoundary ) nBd++;
	if(fabs(trackEndPt2_Y + fDetHalfHeight) < fTPCYBoundary ) nBd++;
	if(fabs(trackEndPt1_Z - fDetLength)<fTPCZBoundary || fabs(trackEndPt2_Z - fDetLength) < fTPCZBoundary ) nBd++;
	if(fabs(trackEndPt1_Z )< fTPCZBoundary || fabs(trackEndPt2_Z )< fTPCZBoundary ) nBd++;
	if( nBd>1 ) isCosmic = 2;
      }

      
      std::vector<float> endPt1;
      std::vector<float> endPt2;
      endPt1.push_back( trackEndPt1_X );
      endPt1.push_back( trackEndPt1_Y );
      endPt1.push_back( trackEndPt1_Z );
      endPt2.push_back( trackEndPt2_X );
      endPt2.push_back( trackEndPt2_Y );
      endPt2.push_back( trackEndPt2_Z );

      float cosmicScore = isCosmic > 0 ? 1 : 0;

      //////////////////////////////
      // Now check for X boundary
      //////////////////////////////
      if( isCosmic==0 ) {
	anab::CosmicTag cctt = anab::CosmicTag(endPt1, endPt2, 0, isCosmic );
	int nXBd =0;
	float xBnd1 = -9999; //cctt.getXInteraction(endPt1[0], 2.0*fDetWidth, fReadOutWindowSize, trackTime, std::floor(tick1) );
	float xBnd2 = -9999; //cctt.getXInteraction(endPt1[0], 2.0*fDetWidth, fReadOutWindowSize, trackTime, std::floor(tick2) );
	if(xBnd1 < fTPCXBoundary || xBnd2 < fTPCXBoundary) nXBd++;
	if( ( fDetWidth - xBnd1 < fTPCXBoundary ) || ( fDetWidth - xBnd1 < fTPCXBoundary ) ) nXBd++;
       	if(  nXBd+nBd>1 && 0 ) isCosmic = 3; // THIS ISN'T SETUP YET -- NEED A HANDLE TO TIME INFO
	if( nBd >0 ) {isCosmic=3; cosmicScore = 0.5;}
      }

      //std::cerr << "Cosmic Score, isCosmic: " << cosmicScore << " " << isCosmic << std::endl;
      cosmicTagTrackVector->push_back( anab::CosmicTag(endPt1,
 						       endPt2,
 						       cosmicScore,
 						       isCosmic
						       ) );


    
//mf::LogInfo("CosmicTrackTagger Results") << "The IsCosmic value is "<< isCosmic << " origin: " << origin
//					  << trackEndPt1_X<<","<< trackEndPt1_Y << "," << trackEndPt1_Z<< " | | " 
//					  << trackEndPt2_X<< ","<< trackEndPt2_Y <<"," << trackEndPt2_Z;
      
 


      //outTracksForTags->push_back( *tTrack );


      util::CreateAssn(*this, e, *cosmicTagTrackVector, tTrack, *assnOutCosmicTagTrack );
      //util::CreateAssn(*this, e, *cosmicTagTrackVector, HitVec, *assnOutCosmicTagHit);


    }
    // END OF LOOPING OVER INSPILL TRACKS






  //  e.put( std::move(outTracksForTags) );

  e.put( std::move(cosmicTagTrackVector) );
  e.put( std::move(assnOutCosmicTagTrack) );




  TrkVec.clear();



} // end of produce
//////////////////////////////////////////////////////////////////////////////////////////////////////





void cosmic::CosmicTrackTagger::beginJob() {



}

void cosmic::CosmicTrackTagger::reconfigure(fhicl::ParameterSet const & p) {
  // Implementation of optional member function here.
  
  ////////  fSptalg                = new cosmic::SpacePointAlg(p.get<fhicl::ParameterSet>("SpacePointAlg"));


  art::ServiceHandle<util::DetectorProperties> detp;
  art::ServiceHandle<util::LArProperties> larp;
  art::ServiceHandle<geo::Geometry> geo;

  fDetHalfHeight = geo->DetHalfHeight();
  fDetWidth      = 2.*geo->DetHalfWidth();
  fDetLength     = geo->DetLength();

  float fSamplingRate = detp->SamplingRate();

  fTrackModuleLabel = p.get< std::string >("TrackModuleLabel", "track");

  fTPCXBoundary = p.get< float >("TPCXBoundary", 5);
  fTPCYBoundary = p.get< float >("TPCYBoundary", 5);
  fTPCZBoundary = p.get< float >("TPCZBoundary", 5);


  const double driftVelocity = larp->DriftVelocity( larp->Efield(), larp->Temperature() ); // cm/us

  //std::cerr << "Drift velocity is " << driftVelocity << " cm/us.  Sampling rate is: "<< fSamplingRate << " detector width: " <<  2*geo->DetHalfWidth() << std::endl;
  fDetectorWidthTicks = 2*geo->DetHalfWidth()/(driftVelocity*fSamplingRate/1000); // ~3200 for uB




}

void cosmic::CosmicTrackTagger::endJob() {
  // Implementation of optional member function here.
}

DEFINE_ART_MODULE(cosmic::CosmicTrackTagger)