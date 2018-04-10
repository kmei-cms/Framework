#ifndef MUON_H
#define MUON_H

class Muon
{
private:
    std::vector<TLorentzVector>* good_muons_;
    std::vector<int>* good_muons_charge_;
    void muon(NTupleReader& tr)
    {
        const auto& allMuons = tr.getVec<TLorentzVector>("Muons");
        const auto& allMuons_passIso = tr.getVec<bool>("Muons_passIso");
        const auto& allMuons_charge = tr.getVec<int>("Muons_charge");
        const auto& etaCut = tr.getVar<double>("etaCut");

        good_muons_ = new std::vector<TLorentzVector>();
        good_muons_charge_ = new std::vector<int>();
        for (unsigned int imu = 0; imu < allMuons.size(); ++imu)
        {
            TLorentzVector lvmu(allMuons.at(imu));
            if( abs(lvmu.Eta()) < etaCut && 
                lvmu.Pt() > 30 && 
                allMuons_passIso.at(imu))
            {
                good_muons_->push_back(lvmu);
                good_muons_charge_->push_back(allMuons_charge.at(imu));
            }
        }


        tr.registerDerivedVec("GoodMuons", good_muons_);
        tr.registerDerivedVar("NGoodMuons", (good_muons_==nullptr)?0:good_muons_->size());
        tr.registerDerivedVec("GoodMuonsCharge", good_muons_charge_);
    }

public:
    Muon() 
        : good_muons_(nullptr)
        , good_muons_charge_(nullptr) 
    {}

    void operator()(NTupleReader& tr)
    {
        muon(tr);
    }
};

#endif