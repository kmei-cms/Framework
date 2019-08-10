#include "Framework/Framework/include/Utility.h"
#include <math.h>

namespace utility
{
    double calcDPhi(const double phi1, const double phi2)
    {
        double dphi = phi1 - phi2 ;
        if ( dphi >  M_PI ) dphi -= 2*M_PI ;
        if ( dphi < -M_PI ) dphi += 2*M_PI ;
        return dphi;
    }
    
    double calcDR(const double eta1, const double eta2, const double phi1, const double phi2)
    {
        const double deta = fabs( eta1 - eta2 ) ;
        double dphi = phi1 - phi2 ;
        if ( dphi > M_PI ) dphi -= 2*M_PI ;
        if ( dphi <-M_PI ) dphi += 2*M_PI ;
        
        return sqrt( dphi*dphi + deta*deta ) ;
    }

    double calcMT(const TLorentzVector& lepton, const TLorentzVector& met)
    {
        // Assuming that both lepton and met are massless
        const double mt_sq = 2 * lepton.Pt() * met.Pt() * ( 1-cos(met.Phi()-lepton.Phi()) );
        return sqrt(mt_sq);
    }    
}
