#ifndef RUNTOPTAGGER_H
#define RUNTOPTAGGER_H

#include "Framework/Framework/include/SetUpTopTagger.h"

class RunTopTagger
{
private:
    std::string myVarSuffix_;

    std::unique_ptr<TopTagger> tt_;
    std::vector<TLorentzVector> hadtops_;
    std::vector<std::vector<const TLorentzVector*>> hadtopdaughters_;
    std::vector<std::vector<int>> hadtopdaughters_id_;
    std::vector<TLorentzVector> neutralinos_;
    std::vector<TLorentzVector> singlinos_;
    std::vector<TLorentzVector> singlets_;
    std::vector<int> hadtops_idx_;
    std::string taggerCfg_;
    int ntops_;
    int ntops_3jet_;
    int ntops_2jet_;
    int ntops_1jet_;
    
    inline int findParent(const int p, const int idx, const std::vector<int>& GenParticles_ParentId, const std::vector<int>& GenParticles_ParentIdx) const
    {
        if (idx == -1)
        {
            return -1;
        }
        else if(abs(GenParticles_ParentId[idx]) == p)
        {
            return GenParticles_ParentIdx[idx];
        }
        else
        {
            return findParent(p, GenParticles_ParentIdx[idx], GenParticles_ParentId, GenParticles_ParentIdx);
        }
    }

    void genMatch(NTupleReader& tr)
    {
        // ----------------------------------------------
        // check for number of hadronic tops at gen level
        // ----------------------------------------------
        const auto& runtype = tr.getVar<std::string>("runtype");

        if(runtype != "Data")
        {
            hadtops_            = tr.createDerivedVec<TLorentzVector>("hadtops"+myVarSuffix_);
            hadtopdaughters_    = tr.createDerivedVec<std::vector<const TLorentzVector*>>("hadtopdaughters"+myVarSuffix_);
            hadtopdaughters_id_ = tr.createDerivedVec<std::vector<int>>("hadtopdaughters_id"+myVarSuffix_);
            neutralinos_        = tr.createDerivedVec<TLorentzVector>("neutralinos"+myVarSuffix_);
            singlinos_          = tr.createDerivedVec<TLorentzVector>("singlinos"+myVarSuffix_);
            singlets_           = tr.createDerivedVec<TLorentzVector>("singlets"+myVarSuffix_);
            hadtops_idx_        = tr.createDerivedVec<int>("hadtops_idx"+myVarSuffix_);
        
            bool genTopEvt      = false;
            bool genTopAcptEvt  = false;
            bool genTopMatchEvt = false;

            const auto& GenParticles            = tr.getVec<TLorentzVector>("GenParticles");
            const auto& GenParticles_PdgId      = tr.getVec<int>("GenParticles_PdgId");
            const auto& GenParticles_ParentId   = tr.getVec<int>("GenParticles_ParentId");
            const auto& GenParticles_ParentIdx  = tr.getVec<int>("GenParticles_ParentIdx");
            const auto& GenParticles_Status     = tr.getVec<int>("GenParticles_Status");            
            const auto& GenParticles_TTFlag     = tr.getVec<bool>("GenParticles_TTFlag");

            for(unsigned int gpi=0; gpi < GenParticles.size(); gpi++ ) 
            {
                int pdgid = GenParticles_PdgId.at(gpi);
                int status = GenParticles_Status.at(gpi);
                int momid = GenParticles_ParentId.at(gpi) ;
                int momidx = GenParticles_ParentIdx.at(gpi);
                int momstatus = (momidx == -1) ? -1 : GenParticles_Status.at(momidx);
                int topIdx = findParent(6, gpi, GenParticles_ParentId, GenParticles_ParentIdx);
                bool passTopStatus = status == 22 || status == 23 || status == 52;
                bool passWMomStatus = false;
                if((abs(momid) == 24) && (momstatus != 1 || status == 2 || momstatus != 22 || momstatus != 23 || momstatus != 52) )
                {
                    passWMomStatus = true;
                }
                else if(abs(pdgid) == 5) passWMomStatus = true;
                //printf(" %6i: status: %6i pdg: %6i motherID: %6i motherIDX: %6i TTFlag: %6i", gpi,  GenParticles_Status[gpi], GenParticles_PdgId[gpi], GenParticles_ParentId[gpi], GenParticles_ParentIdx[gpi], int(GenParticles_TTFlag[gpi]) ); fflush(stdout);
                if(abs(pdgid) == 1000022 && (status==22 || status == 52))
                {
                    neutralinos_.push_back(GenParticles.at(gpi));
                }
                if(abs(pdgid) == 5000001 && (status == 22 || status == 52))
                {
                    singlinos_.push_back(GenParticles.at(gpi));
                }
                if(abs(pdgid) == 5000002 && (status == 22 || status == 52))
                {
                    singlets_.push_back(GenParticles.at(gpi));
                }
                if( topIdx >= 0 && (abs(pdgid) != 24) && passTopStatus && passWMomStatus)
                {
                    //printf(" topIdx: %i particle: %i\n", topIdx, pdgid); fflush(stdout);
                    
                    int position = 0;
                    for(;position < hadtops_idx_.size() && hadtops_idx_[position] != topIdx; ++position);
                    if( position < hadtops_idx_.size() )
                    {
                        hadtopdaughters_[position].push_back(&(GenParticles.at(gpi)));
                        hadtopdaughters_id_[position].push_back(gpi);
                    } 
                    else
                    {
                        hadtops_idx_.push_back(topIdx);
                        hadtops_.push_back(GenParticles.at(topIdx));

                        std::vector<const TLorentzVector*> daughters;
                        daughters.push_back(&(GenParticles.at(gpi)));
                        hadtopdaughters_.push_back(daughters);

                        std::vector<int> daughters_id;
                        daughters_id.push_back(gpi);
                        hadtopdaughters_id_.push_back(daughters_id);
                    }
                }
                //else
                //{
                //    printf("\n");
                //}
            }            
        }
    }

    void countTops(const std::vector<TopObject*>& tops)
    {
        ntops_ = 0;
        ntops_3jet_ = 0;
        ntops_2jet_ = 0;
        ntops_1jet_ = 0;
       
        for (const TopObject* top : tops)
        {
            ntops_++;

            if(top->getNConstituents() == 3 )
            {
                ntops_3jet_++;
            }
            else if(top->getNConstituents() == 2 )
            {
                ntops_2jet_++;
            }
            else if(top->getNConstituents() == 1 )
            {
                ntops_1jet_++;
            }
        }
    }

    void runTopTagger(NTupleReader& tr)
    {
        // Prepare class to run Top Tagger
        genMatch(tr);

        // Setup variables needed for top tagger
        SetUpTopTagger st( tr, hadtops_, hadtopdaughters_, myVarSuffix_ );
        const std::vector<Constituent>& constituents = st.getConstituents();

        // Run the top tagger             
        tt_->runTagger(constituents);

        // Get the top tagger results object     
        const TopTaggerResults& ttr = tt_->getResults();
        
        // For now we will only use merged and resolved tops
        const std::vector<TopObject*>& taggedObjects = ttr.getTops();
        std::vector<TopObject*> mergedTops;
        std::vector<TopObject*> resolvedTops;
        for(auto* o : taggedObjects)
        {
            if     (o->getType()==TopObject::MERGED_TOP)   mergedTops.push_back(o);
            else if(o->getType()==TopObject::RESOLVED_TOP) resolvedTops.push_back(o);
        }

        // Get reconstructed tops and derive needed variables                            
        std::vector<TopObject*> tops(mergedTops);
        tops.insert(tops.end(), resolvedTops.begin(), resolvedTops.end());
        countTops(tops);
        
        // -------------------------------------
        // -- Calculate DeltaR between 2 tops
        // -------------------------------------
        double dR_top1_top2 = -1;
        if(ntops_ == 2)
        {
            dR_top1_top2 = tops.at(0)->p().DeltaR(tops.at(1)->p()); 
        }

        // -------------------------------------------------------------
        // -- get vectors of the top mass & eta & pT & LV in the event
        // -------------------------------------------------------------
        auto& topsMass = tr.createDerivedVec<double>("topsMass"+myVarSuffix_);
        auto& topsEta  = tr.createDerivedVec<double>("topsEta"+myVarSuffix_);
        auto& topsPt   = tr.createDerivedVec<double>("topsPt"+myVarSuffix_);
        auto& topsLV   = tr.createDerivedVec<TLorentzVector>("topsLV"+myVarSuffix_);
        for(const auto* t : tops)
        {
            topsMass.push_back(t->p().M());
            topsEta.push_back(t->p().Eta());
            topsPt.push_back(t->p().Pt());
            topsLV.push_back(t->p());
        }

        // ----------------------------------
        // -- get best top mass & eta & pt 
        // ----------------------------------
        double bestTopMass             = -9999.9;
        double bestTopEta              = -9999.9;
        double bestTopPt               = -9999.9;
        const TopObject* bestTopMassLV = nullptr;
        bool bestTopMassGenMatch       = false;
        bool bestTopMassTopTag         = false;
        for(int iTop = 0; iTop < tops.size(); ++iTop)
        {
            auto* top = tops[iTop];

            if(fabs(top->p().M() - 173.21) < fabs(bestTopMass - 173.21))
            {
                bestTopMass = top->p().M();
                bestTopEta = top->p().Eta();
                bestTopPt = top->p().Pt();
                bestTopMassLV = top;
            }     
        }
        bestTopMassGenMatch = (bestTopMassLV)?(bestTopMassLV->getBestGenTopMatch(0.6) != nullptr):(false);
        
        for(const auto& topPtr : tops)
        {
            if(topPtr == bestTopMassLV)
            {
                bestTopMassTopTag = true;
                break;
            }
        }

        // -----------------------------------------------
        // -- get variables for fake rate & efficiency 
        // -----------------------------------------------
        const auto& candidateTops          = ttr.getTopCandidates();
        float highestDisc                  = -9999.9;
        float bestTopMassCand              = -9999.9;
        float bestTopEtaCand               = -9999.9;
        const TopObject* bestTopMassLVCand = nullptr;
        float bestTopMassTopTagDisc        = -999.9;
        bool bestTopMassGenMatchCand       = false;
        bool bestTopMassTopTagCand         = false;
        for(int iTop = 0; iTop < candidateTops.size(); ++iTop)
            {
                auto& top = candidateTops[iTop];

                highestDisc = (top.getDiscriminator() > highestDisc ? top.getDiscriminator() : highestDisc);

                if(fabs(top.p().M() - 173.5) < fabs(bestTopMassCand - 173.5) && top.getNConstituents() == 3)
                {
                    bestTopMassCand = top.p().M();
                    bestTopEtaCand = top.p().Eta();
                    bestTopMassLVCand = &top;
                    bestTopMassTopTagDisc = top.getDiscriminator();
                }
        }
        bestTopMassGenMatchCand = (bestTopMassLVCand)?(bestTopMassLVCand->getBestGenTopMatch(0.6) != nullptr):(false);

        for(const auto& topPtr : tops) 
        {
            if(topPtr == bestTopMassLVCand) 
            {
                bestTopMassTopTagCand = true;
                break;
            }
        }
        
        // Making tight photon lv (should live somewhere else: is needed for HistoContainer.h)
        const auto& Photons        = tr.getVec<TLorentzVector>("Photons");
        const auto& Photons_fullID = tr.getVec<bool>("Photons_fullID");

        auto& tightPhotons = tr.createDerivedVec<TLorentzVector>("tightPhotons"+myVarSuffix_);
        for(int i = 0; i < Photons.size(); ++i)
        {
            if(Photons_fullID[i])
            {
                tightPhotons.push_back(Photons[i]);
            }
        }
        
        //std::cout<<" Size Yo "<<hadtops_.size()<<"  "<<hadtopdaughters_.size()<<std::endl;
        //for (int i = 0; i < hadtops_.size(); ++i)
        //{
        //    TLorentzVector dSum;
        //    for (int j = 0; j < (hadtopdaughters_[i]).size(); j++)
        //    {
        //        dSum += *((hadtopdaughters_[i])[j]);
        //    }
        //    printf("nTops: %i ndaughters %i   top: (pt %4.5lf , eta %4.5lf, phi %4.5lf, mass %4.5lf) dSum: (pt %4.5lf , eta %4.5lf, phi %4.5lf, mass %4.5lf)\n", hadtops_.size(), hadtopdaughters_[i].size(), 
        //           hadtops_[i].Pt(), hadtops_[i].Eta(), hadtops_[i].Phi(), hadtops_[i].M(), 
        //           dSum.Pt(), dSum.Eta(), dSum.Phi(), dSum.M()
        //          );
        //
        //}
        //
        //printf("=========================================================================================\n");

        // Register Variables
        tr.registerDerivedVar("ttr"+myVarSuffix_, &ttr);
        tr.registerDerivedVar("ntops"+myVarSuffix_, ntops_);
        tr.registerDerivedVar("ntops_3jet"+myVarSuffix_, ntops_3jet_);
        tr.registerDerivedVar("ntops_2jet"+myVarSuffix_, ntops_2jet_);
        tr.registerDerivedVar("ntops_1jet"+myVarSuffix_, ntops_1jet_);
        tr.registerDerivedVar("dR_top1_top2"+myVarSuffix_,dR_top1_top2);
        tr.registerDerivedVar("bestTopMass"+myVarSuffix_, bestTopMass);
        tr.registerDerivedVar("bestTopEta"+myVarSuffix_, bestTopEta);
        tr.registerDerivedVar("bestTopPt"+myVarSuffix_, bestTopPt);
        tr.registerDerivedVar("bestTopMassLV"+myVarSuffix_, bestTopMassLV?(bestTopMassLV->p()):(TLorentzVector()));
        tr.registerDerivedVar("bestTopMassGenMatch"+myVarSuffix_, bestTopMassGenMatch);
        tr.registerDerivedVar("bestTopMassTopTag"+myVarSuffix_, bestTopMassTopTag);
        tr.registerDerivedVar("highestDisc"+myVarSuffix_, highestDisc);
        tr.registerDerivedVar("bestTopMassCand"+myVarSuffix_, bestTopMassCand);
        tr.registerDerivedVar("bestTopEtaCand"+myVarSuffix_, bestTopEtaCand);
        tr.registerDerivedVar("bestTopMassLVCand"+myVarSuffix_, bestTopMassLVCand?(bestTopMassLVCand->p()):(TLorentzVector()));
        tr.registerDerivedVar("bestTopMassTopTagDisc"+myVarSuffix_, bestTopMassTopTagDisc);
        tr.registerDerivedVar("bestTopMassGenMatchCand"+myVarSuffix_, bestTopMassGenMatchCand);
        tr.registerDerivedVar("bestTopMassTopTagCand"+myVarSuffix_, bestTopMassTopTagCand); 
    }

public:
    RunTopTagger(std::string taggerCfg = "TopTagger.cfg", std::string myVarSuffix = "") 
        : taggerCfg_         (taggerCfg)
        , myVarSuffix_       (myVarSuffix)
        , tt_                (new TopTagger())
    {                
        std::cout<<"Setting up RunTopTagger"<<std::endl;
        tt_->setCfgFile(taggerCfg_);
        std::cout<<"Using "+taggerCfg+" as the TopTagger config file"<<std::endl;
    }

    void operator()(NTupleReader& tr)
    {
        runTopTagger(tr);
    }
};

#endif
