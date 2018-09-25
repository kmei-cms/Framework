#ifndef ELECTRON_H
#define ELECTRON_H

class Electron
{
private:
    void electron(NTupleReader& tr)
    {
        const auto& allElectrons = tr.getVec<TLorentzVector>("Electrons");
        const auto& allElectrons_passIso = tr.getVec<bool>("Electrons_passIso");
        const auto& allElectrons_charge  = tr.getVec<int>("Electrons_charge");
        const auto& allElectrons_tightID = tr.getVec<bool>("Electrons_tightID");
        const auto& etaCut = tr.getVar<double>("etaCut");
        const auto& MET = tr.getVar<double>("MET"); 
        const auto& METPhi = tr.getVar<double>("METPhi");

        TLorentzVector lvMET;
        lvMET.SetPtEtaPhiM(MET, 0.0, METPhi, 0.0);

        auto* good_electrons_ = new std::vector<bool>();
        auto* electrons_mtw_ = new std::vector<double>();
        int NGoodElectrons = 0;
        for(unsigned int iel = 0; iel < allElectrons.size(); ++iel)
        {
            TLorentzVector lvel = allElectrons.at(iel);
            double mtw = sqrt( 2*( lvMET.Pt()*lvel.Pt() - (lvMET.Px()*lvel.Px() + lvMET.Py()*lvel.Py()) ) );
            electrons_mtw_->push_back(mtw);
            if( abs(lvel.Eta()) < etaCut && 
                lvel.Pt() > 30 && 
                allElectrons_passIso.at(iel) &&
                allElectrons_tightID.at(iel) 
                )
            {
                good_electrons_->push_back(true);
                NGoodElectrons++;
            }
            else
            {
                good_electrons_->push_back(false);
            }
        }

        tr.registerDerivedVec("GoodElectrons", good_electrons_);
        tr.registerDerivedVar("NGoodElectrons", NGoodElectrons);
        tr.registerDerivedVec("ElectronsMTW", electrons_mtw_);
    }

public:
    Electron() 
    {
        std::cout<<"Setting up Electron"<<std::endl;   
    }

    void operator()(NTupleReader& tr)
    {
        electron(tr);
    }
};

#endif
