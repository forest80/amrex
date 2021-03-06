
#include <AMReX_MultiFabUtil.H>
#include <AMReX_MultiFabUtil_F.H>
#include <AMReX_MultiFabUtil_C.H>
#include <sstream>
#include <iostream>

namespace {

    using namespace amrex;

    static Box
    getIndexBox(const RealBox& real_box, const Geometry& geom) {
        IntVect slice_lo, slice_hi;

        AMREX_D_TERM(slice_lo[0]=floor((real_box.lo(0) - geom.ProbLo(0))/geom.CellSize(0));,
                     slice_lo[1]=floor((real_box.lo(1) - geom.ProbLo(1))/geom.CellSize(1));,
                     slice_lo[2]=floor((real_box.lo(2) - geom.ProbLo(2))/geom.CellSize(2)););

        AMREX_D_TERM(slice_hi[0]=floor((real_box.hi(0) - geom.ProbLo(0))/geom.CellSize(0));,
                     slice_hi[1]=floor((real_box.hi(1) - geom.ProbLo(1))/geom.CellSize(1));,
                     slice_hi[2]=floor((real_box.hi(2) - geom.ProbLo(2))/geom.CellSize(2)););

        return Box(slice_lo, slice_hi) & geom.Domain();
    }

    static
    std::unique_ptr<MultiFab> allocateSlice(int dir, const MultiFab& cell_centered_data,
                                            int ncomp, const Geometry& geom, Real dir_coord,
                                            Vector<int>& slice_to_full_ba_map) {

        // Get our slice and convert to index space
        RealBox real_slice = geom.ProbDomain();
        real_slice.setLo(dir, dir_coord);
        real_slice.setHi(dir, dir_coord);
        Box slice_box = getIndexBox(real_slice, geom);

        // define the multifab that stores slice
        BoxArray ba = cell_centered_data.boxArray();
        const DistributionMapping& dm = cell_centered_data.DistributionMap();
        std::vector< std::pair<int, Box> > isects;
        ba.intersections(slice_box, isects, false, 0);
        Vector<Box> boxes;
        Vector<int> procs;
        for (int i = 0; i < isects.size(); ++i) {
            procs.push_back(dm[isects[i].first]);
            boxes.push_back(isects[i].second);
            slice_to_full_ba_map.push_back(isects[i].first);
        }
        BoxArray slice_ba(&boxes[0], boxes.size());
        DistributionMapping slice_dmap(std::move(procs));
        std::unique_ptr<MultiFab> slice(new MultiFab(slice_ba, slice_dmap, ncomp, 0,
                                                     MFInfo(), cell_centered_data.Factory()));
        return slice;
    }
}

namespace amrex
{
    void average_node_to_cellcenter (MultiFab& cc, int dcomp,
         const MultiFab& nd, int scomp, int ncomp, int ngrow)
    {
#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
        for (MFIter mfi(cc,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box bx = mfi.growntilebox(ngrow);
            FArrayBox* ccfab = &(cc[mfi]);
            FArrayBox const* ndfab = &(nd[mfi]);

            AMREX_LAUNCH_HOST_DEVICE_LAMBDA ( bx, tbx,
            {
                amrex_avg_nd_to_cc(tbx, *ccfab, *ndfab, dcomp, scomp, ncomp);
            });
        }
    }

    void average_edge_to_cellcenter (MultiFab& cc, int dcomp,
        const Vector<const MultiFab*>& edge, int ngrow)
    {
        AMREX_ASSERT(cc.nComp() >= dcomp + AMREX_SPACEDIM);
        AMREX_ASSERT(edge.size() == AMREX_SPACEDIM);
        AMREX_ASSERT(edge[0]->nComp() == 1);
#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
        for (MFIter mfi(cc,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box bx = mfi.growntilebox(ngrow);
            FArrayBox* ccfab = &(cc[mfi]);
            AMREX_D_TERM(FArrayBox const* exfab = &((*edge[0])[mfi]);,
                         FArrayBox const* eyfab = &((*edge[1])[mfi]);,
                         FArrayBox const* ezfab = &((*edge[2])[mfi]););

            AMREX_LAUNCH_HOST_DEVICE_LAMBDA ( bx, tbx,
            {
                amrex_avg_eg_to_cc(tbx, *ccfab, AMREX_D_DECL(*exfab,*eyfab,*ezfab), dcomp);
            });
        }
    }

    void average_face_to_cellcenter (MultiFab& cc, int dcomp,
        const Vector<const MultiFab*>& fc, int ngrow)
    {
        average_face_to_cellcenter(cc, dcomp,
            Array<MultiFab const*,AMREX_SPACEDIM>{AMREX_D_DECL(fc[0],fc[1],fc[2])},
            ngrow);
    }

    void average_face_to_cellcenter (MultiFab& cc, const Vector<const MultiFab*>& fc,
				     const Geometry& geom)
    {
        average_face_to_cellcenter(cc,
                                   Array<MultiFab const*,AMREX_SPACEDIM>
                                                  {AMREX_D_DECL(fc[0],fc[1],fc[2])},
                                   geom);
    }

    void average_face_to_cellcenter (MultiFab& cc, int dcomp,
        const Array<const MultiFab*,AMREX_SPACEDIM>& fc, int ngrow)
    {
        AMREX_ASSERT(cc.nComp() >= dcomp + AMREX_SPACEDIM);
        AMREX_ASSERT(fc[0]->nComp() == 1);

#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
        for (MFIter mfi(cc,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box bx = mfi.growntilebox(ngrow);
            FArrayBox* ccfab = &(cc[mfi]);
            AMREX_D_TERM(FArrayBox const* fxfab = &((*fc[0])[mfi]);,
                         FArrayBox const* fyfab = &((*fc[1])[mfi]);,
                         FArrayBox const* fzfab = &((*fc[2])[mfi]););

            AMREX_LAUNCH_HOST_DEVICE_LAMBDA ( bx, tbx,
            {
                amrex_avg_fc_to_cc(tbx, *ccfab, AMREX_D_DECL(*fxfab,*fyfab,*fzfab), dcomp, GeometryData());
            });
        }
    }

    void average_face_to_cellcenter (MultiFab& cc,
                                     const Array<const MultiFab*,AMREX_SPACEDIM>& fc,
				     const Geometry& geom)
    {
	AMREX_ASSERT(cc.nComp() >= AMREX_SPACEDIM);
	AMREX_ASSERT(fc[0]->nComp() == 1); // We only expect fc to have the gradient perpendicular to the face

        const GeometryData gd = geom.data();

#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
        for (MFIter mfi(cc,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box bx = mfi.tilebox();
            FArrayBox* ccfab = &(cc[mfi]);
            AMREX_D_TERM(FArrayBox const* fxfab = &((*fc[0])[mfi]);,
                         FArrayBox const* fyfab = &((*fc[1])[mfi]);,
                         FArrayBox const* fzfab = &((*fc[2])[mfi]););

            AMREX_LAUNCH_HOST_DEVICE_LAMBDA ( bx, tbx,
            {
                amrex_avg_fc_to_cc(tbx, *ccfab, AMREX_D_DECL(*fxfab,*fyfab,*fzfab), 0, gd);
            });
        }
    }

    void average_cellcenter_to_face (const Vector<MultiFab*>& fc, const MultiFab& cc,
				     const Geometry& geom)
    {
        average_cellcenter_to_face(Array<MultiFab*,AMREX_SPACEDIM>{AMREX_D_DECL(fc[0],fc[1],fc[2])},
                                   cc, geom);
    }


    void average_cellcenter_to_face (const Array<MultiFab*,AMREX_SPACEDIM>& fc, const MultiFab& cc,
                                    const Geometry& geom)
    {
	AMREX_ASSERT(cc.nComp() == 1);
	AMREX_ASSERT(cc.nGrow() >= 1);
	AMREX_ASSERT(fc[0]->nComp() == 1); // We only expect fc to have the gradient perpendicular to the face

        const GeometryData& gd = geom.data();

#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
	for (MFIter mfi(cc,TilingIfNotGPU()); mfi.isValid(); ++mfi)
	{
            AMREX_D_TERM(const Box& xbx = mfi.nodaltilebox(0);,
                         const Box& ybx = mfi.nodaltilebox(1);,
                         const Box& zbx = mfi.nodaltilebox(2););
            const auto& index_bounds = amrex::getIndexBounds(AMREX_D_DECL(xbx,ybx,zbx));

            AMREX_D_TERM(FArrayBox* fxfab = &((*fc[0])[mfi]);,
                         FArrayBox* fyfab = &((*fc[1])[mfi]);,
                         FArrayBox* fzfab = &((*fc[2])[mfi]););
            FArrayBox const* ccfab = &(cc[mfi]);
            
            AMREX_LAUNCH_HOST_DEVICE_LAMBDA (index_bounds, tbx,
            {
                amrex_avg_cc_to_fc(tbx, AMREX_D_DECL(xbx,ybx,zbx),
                                   AMREX_D_DECL(*fxfab,*fyfab,*fzfab), *ccfab, gd);
            });
	}
    }

// *************************************************************************************************************

    // Average fine cell-based MultiFab onto crse cell-centered MultiFab.
    // We do NOT assume that the coarse layout is a coarsened version of the fine layout.
    // This version DOES use volume-weighting.
    void average_down (const MultiFab& S_fine, MultiFab& S_crse,
		       const Geometry& fgeom, const Geometry& cgeom,
                       int scomp, int ncomp, int rr)
    {
         average_down(S_fine,S_crse,fgeom,cgeom,scomp,ncomp,rr*IntVect::TheUnitVector());
    }

    void average_down (const MultiFab& S_fine, MultiFab& S_crse,
		       const Geometry& fgeom, const Geometry& cgeom,
                       int scomp, int ncomp, const IntVect& ratio)
    {

        if (S_fine.is_nodal() || S_crse.is_nodal())
        {
            amrex::Error("Can't use amrex::average_down for nodal MultiFab!");
        }

#if (AMREX_SPACEDIM == 3)
	amrex::average_down(S_fine, S_crse, scomp, ncomp, ratio);
	return;
#else

        AMREX_ASSERT(S_crse.nComp() == S_fine.nComp());

        //
        // Coarsen() the fine stuff on processors owning the fine data.
        //
	const BoxArray& fine_BA = S_fine.boxArray();
	const DistributionMapping& fine_dm = S_fine.DistributionMap();
        BoxArray crse_S_fine_BA = fine_BA;
	crse_S_fine_BA.coarsen(ratio);

        MultiFab crse_S_fine(crse_S_fine_BA,fine_dm,ncomp,0,MFInfo(),FArrayBoxFactory());

	MultiFab fvolume;
	fgeom.GetVolume(fvolume, fine_BA, fine_dm, 0);

#ifdef _OPENMP
#pragma omp parallel
#endif
        for (MFIter mfi(crse_S_fine,true); mfi.isValid(); ++mfi)
        {
            //  NOTE: The tilebox is defined at the coarse level.
            const Box& tbx = mfi.tilebox();

            //  NOTE: We copy from component scomp of the fine fab into component 0 of the crse fab
            //        because the crse fab is a temporary which was made starting at comp 0, it is
            //        not part of the actual crse multifab which came in.
            BL_FORT_PROC_CALL(BL_AVGDOWN_WITH_VOL,bl_avgdown_with_vol)
                (tbx.loVect(), tbx.hiVect(),
                 BL_TO_FORTRAN_N(S_fine[mfi],scomp),
                 BL_TO_FORTRAN_N(crse_S_fine[mfi],0),
                 BL_TO_FORTRAN(fvolume[mfi]),
                 ratio.getVect(),&ncomp);
	}

        S_crse.copy(crse_S_fine,0,scomp,ncomp);
#endif
   }

// *************************************************************************************************************

    // Average fine cell-based MultiFab onto crse cell-centered MultiFab.
    // We do NOT assume that the coarse layout is a coarsened version of the fine layout.
    // This version does NOT use volume-weighting
    void average_down (const MultiFab& S_fine, MultiFab& S_crse, int scomp, int ncomp, int rr)
    {
         average_down(S_fine,S_crse,scomp,ncomp,rr*IntVect::TheUnitVector());
    }


    void sum_fine_to_coarse(const MultiFab& S_fine, MultiFab& S_crse,
                            int scomp, int ncomp, const IntVect& ratio,
                            const Geometry& cgeom, const Geometry& fgeom)
    {
        AMREX_ASSERT(S_crse.nComp() == S_fine.nComp());
        AMREX_ASSERT(ratio == ratio[0]);
        AMREX_ASSERT(S_fine.nGrow() % ratio[0] == 0);

        const int nGrow = S_fine.nGrow() / ratio[0];

        //
        // Coarsen() the fine stuff on processors owning the fine data.
        //
        BoxArray crse_S_fine_BA = S_fine.boxArray(); crse_S_fine_BA.coarsen(ratio);

        MultiFab crse_S_fine(crse_S_fine_BA, S_fine.DistributionMap(), ncomp, nGrow, MFInfo(), FArrayBoxFactory());

#ifdef _OPENMP
#pragma omp parallel
#endif
        for (MFIter mfi(crse_S_fine, true); mfi.isValid(); ++mfi)
        {
            //  NOTE: The tilebox is defined at the coarse level.
            const Box& tbx = mfi.growntilebox(nGrow);

            BL_FORT_PROC_CALL(BL_AVGDOWN, bl_avgdown)
                (tbx.loVect(), tbx.hiVect(),
                 BL_TO_FORTRAN_N(S_fine[mfi] , scomp),
                 BL_TO_FORTRAN_N(crse_S_fine[mfi], 0),
                 ratio.getVect(),&ncomp);
        }

        S_crse.copy(crse_S_fine, 0, scomp, ncomp, nGrow, 0,
                    cgeom.periodicity(), FabArrayBase::ADD);
    }

    void average_down (const MultiFab& S_fine, MultiFab& S_crse,
                       int scomp, int ncomp, const IntVect& ratio)
    {
        AMREX_ASSERT(S_crse.nComp() == S_fine.nComp());
        AMREX_ASSERT((S_crse.is_cell_centered() && S_fine.is_cell_centered()) ||
                     (S_crse.is_nodal()         && S_fine.is_nodal()));

        bool is_cell_centered = S_crse.is_cell_centered();

        //
        // Coarsen() the fine stuff on processors owning the fine data.
        //
        BoxArray crse_S_fine_BA = S_fine.boxArray(); crse_S_fine_BA.coarsen(ratio);

        if (crse_S_fine_BA == S_crse.boxArray() and S_fine.DistributionMap() == S_crse.DistributionMap())
        {
#ifdef _OPENMP
#pragma omp parallel
#endif
            for (MFIter mfi(S_crse,true); mfi.isValid(); ++mfi)
            {
                //  NOTE: The tilebox is defined at the coarse level.
                const Box& tbx = mfi.tilebox();

                if (is_cell_centered) {
                    BL_FORT_PROC_CALL(BL_AVGDOWN,bl_avgdown)
                        (tbx.loVect(), tbx.hiVect(),
                         BL_TO_FORTRAN_N(S_fine[mfi],scomp),
                         BL_TO_FORTRAN_N(S_crse[mfi],scomp),
                         ratio.getVect(),&ncomp);
                } else {
                    BL_FORT_PROC_CALL(BL_AVGDOWN_NODES,bl_avgdown_nodes)
                        (tbx.loVect(),tbx.hiVect(),
                         BL_TO_FORTRAN_N(S_fine[mfi],scomp),
                         BL_TO_FORTRAN_N(S_crse[mfi],scomp),
                         ratio.getVect(),&ncomp);
                }
            }
        }
        else
        {
            MultiFab crse_S_fine(crse_S_fine_BA, S_fine.DistributionMap(), ncomp, 0, MFInfo(), FArrayBoxFactory());

#ifdef _OPENMP
#pragma omp parallel
#endif
            for (MFIter mfi(crse_S_fine,true); mfi.isValid(); ++mfi)
            {
                //  NOTE: The tilebox is defined at the coarse level.
                const Box& tbx = mfi.tilebox();

                //  NOTE: We copy from component scomp of the fine fab into component 0 of the crse fab
                //        because the crse fab is a temporary which was made starting at comp 0, it is
                //        not part of the actual crse multifab which came in.

                if (is_cell_centered) {
                    BL_FORT_PROC_CALL(BL_AVGDOWN,bl_avgdown)
                        (tbx.loVect(), tbx.hiVect(),
                         BL_TO_FORTRAN_N(S_fine[mfi],scomp),
                         BL_TO_FORTRAN_N(crse_S_fine[mfi],0),
                         ratio.getVect(),&ncomp);
                } else {
                    BL_FORT_PROC_CALL(BL_AVGDOWN_NODES,bl_avgdown_nodes)
                        (tbx.loVect(), tbx.hiVect(),
                         BL_TO_FORTRAN_N(S_fine[mfi],scomp),
                         BL_TO_FORTRAN_N(crse_S_fine[mfi],0),
                         ratio.getVect(),&ncomp);
                }
            }

            S_crse.copy(crse_S_fine,0,scomp,ncomp);
        }
   }

// *************************************************************************************************************

    void average_down_faces (const Vector<const MultiFab*>& fine,
                             const Vector<MultiFab*>& crse,
                             const IntVect& ratio, int ngcrse)
    {
        average_down_faces(Array<const MultiFab*,AMREX_SPACEDIM>
                                   {AMREX_D_DECL(fine[0],fine[1],fine[2])},
                           Array<MultiFab*,AMREX_SPACEDIM>
                                   {AMREX_D_DECL(crse[0],crse[1],crse[2])},
                           ratio, ngcrse);
    }

    void average_down_faces (const Vector<const MultiFab*>& fine,
                             const Vector<MultiFab*>& crse, int ratio, int ngcrse)
    {
        average_down_faces(fine,crse,IntVect{ratio},ngcrse);
    }

    void average_down_faces (const Array<const MultiFab*,AMREX_SPACEDIM>& fine,
                             const Array<MultiFab*,AMREX_SPACEDIM>& crse,
                             int ratio, int ngcrse)
    {
        average_down_faces(fine,crse,IntVect{ratio},ngcrse);
    }

    // Average fine face-based MultiFab onto crse face-based MultiFab.
    void average_down_faces (const Array<const MultiFab*,AMREX_SPACEDIM>& fine,
                             const Array<MultiFab*,AMREX_SPACEDIM>& crse,
			     const IntVect& ratio, int ngcrse)
    {
	AMREX_ASSERT(crse[0]->nComp() == fine[0]->nComp());

	int ncomp = crse[0]->nComp();

        if (isMFIterSafe(*fine[0], *crse[0]))
        {
#ifdef _OPENMP
#pragma omp parallel
#endif
            for (int n=0; n<AMREX_SPACEDIM; ++n) {
                for (MFIter mfi(*crse[n],true); mfi.isValid(); ++mfi)
                {
                    const Box& tbx = mfi.growntilebox(ngcrse);

                    BL_FORT_PROC_CALL(BL_AVGDOWN_FACES,bl_avgdown_faces)
                        (tbx.loVect(),tbx.hiVect(),
                         BL_TO_FORTRAN((*fine[n])[mfi]),
                         BL_TO_FORTRAN((*crse[n])[mfi]),
                         ratio.getVect(),n,ncomp);
                }
            }
        }
        else
        {
            std::array<MultiFab,AMREX_SPACEDIM> ctmp;
            for (int idim = 0; idim < AMREX_SPACEDIM; ++idim)
            {
                BoxArray cba = fine[idim]->boxArray();
                cba.coarsen(ratio);
                ctmp[idim].define(cba, fine[idim]->DistributionMap(), ncomp, ngcrse, MFInfo(), FArrayBoxFactory());
            }
            average_down_faces(fine, amrex::GetArrOfPtrs(ctmp), ratio, ngcrse);
            for (int idim = 0; idim < AMREX_SPACEDIM; ++idim)
            {
                crse[idim]->ParallelCopy(ctmp[idim],0,0,ncomp,ngcrse,ngcrse);
            }
        }
    }

    //! Average fine edge-based MultiFab onto crse edge-based MultiFab.
    //! This routine assumes that the crse BoxArray is a coarsened version of the fine BoxArray.
    void average_down_edges (const Vector<const MultiFab*>& fine, const Vector<MultiFab*>& crse,
                             const IntVect& ratio, int ngcrse)
    {
	AMREX_ASSERT(crse.size()  == AMREX_SPACEDIM);
	AMREX_ASSERT(fine.size()  == AMREX_SPACEDIM);
	AMREX_ASSERT(crse[0]->nComp() == fine[0]->nComp());

	int ncomp = crse[0]->nComp();

#ifdef _OPENMP
#pragma omp parallel
#endif
        for (int n=0; n<AMREX_SPACEDIM; ++n) {
            for (MFIter mfi(*crse[n],true); mfi.isValid(); ++mfi)
            {
                const Box& tbx = mfi.growntilebox(ngcrse);

                BL_FORT_PROC_CALL(BL_AVGDOWN_EDGES,bl_avgdown_edges)
                    (tbx.loVect(),tbx.hiVect(),
                     BL_TO_FORTRAN((*fine[n])[mfi]),
                     BL_TO_FORTRAN((*crse[n])[mfi]),
                     ratio.getVect(),n,ncomp);
            }
        }
    }

    //! Average fine node-based MultiFab onto crse node-based MultiFab.
    //! This routine assumes that the crse BoxArray is a coarsened version of the fine BoxArray.
    void average_down_nodal (const MultiFab& fine, MultiFab& crse, const IntVect& ratio, int ngcrse)
    {
        AMREX_ASSERT(fine.is_nodal());
        AMREX_ASSERT(crse.is_nodal());
	AMREX_ASSERT(crse.nComp() == fine.nComp());

	int ncomp = crse.nComp();

#ifdef _OPENMP
#pragma omp parallel
#endif
        for (MFIter mfi(crse,true); mfi.isValid(); ++mfi)
            {
                const Box& tbx = mfi.growntilebox(ngcrse);

                BL_FORT_PROC_CALL(BL_AVGDOWN_NODES,bl_avgdown_nodes)
                    (tbx.loVect(),tbx.hiVect(),
                     BL_TO_FORTRAN(fine[mfi]),
                     BL_TO_FORTRAN(crse[mfi]),
                     ratio.getVect(),&ncomp);
            }
    }


    void print_state(const MultiFab& mf, const IntVect& cell, const int n)
    {


#ifdef _OPENMP
#pragma omp parallel
#endif
      for (MFIter mfi(mf); mfi.isValid(); ++mfi)
	{
	  if (mfi.validbox().contains(cell)) {
	    if (n >= 0) {
	      amrex::AllPrint().SetPrecision(17) << mf[mfi](cell, n) << std::endl;
	    } else {
	      std::ostringstream ss;
	      ss.precision(17);
	      const int ncomp = mf.nComp();
	      for (int i = 0; i < ncomp-1; ++i)
		{
		  ss << mf[mfi](cell,i) << ", ";
		}
	      ss << mf[mfi](cell,ncomp-1);
	      amrex::AllPrint() << ss.str() << std::endl;
	    }
	  }
	}

    }

    void writeFabs (const MultiFab& mf, const std::string& name)
    {
        writeFabs (mf, 0, mf.nComp(), name);
    }

    void writeFabs (const MultiFab& mf, int comp, int ncomp, const std::string& name)
    {
        for (MFIter mfi(mf); mfi.isValid(); ++mfi) {
            std::ofstream ofs(name+"-fab-"+std::to_string(mfi.index()));
            mf[mfi].writeOn(ofs, comp, ncomp);
        }
    }

    MultiFab ToMultiFab (const iMultiFab& imf)
    {
        MultiFab mf(imf.boxArray(), imf.DistributionMap(), imf.nComp(), imf.nGrow(),
                    MFInfo(), FArrayBoxFactory());

        const int ncomp = imf.nComp();
#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
        for (MFIter mfi(mf,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box& gbx = mfi.growntilebox();
            FArrayBox      * rfab = &( mf[mfi]);
            IArrayBox const* ifab = &(imf[mfi]);
            AMREX_LAUNCH_HOST_DEVICE_LAMBDA (gbx, tbx,
            {
                amrex::cast(tbx, *rfab, *ifab, 0, 0, ncomp);
            });
        }

        return mf;
    }

    FabArray<BaseFab<long> > ToLongMultiFab (const iMultiFab& imf)
    {
        FabArray<BaseFab<long> > lmf(imf.boxArray(), imf.DistributionMap(), imf.nComp(), imf.nGrow());

        const int ncomp = imf.nComp();
#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
        for (MFIter mfi(lmf,TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            const Box& gbx = mfi.growntilebox();
            BaseFab<long>  * lfab = &(lmf[mfi]);
            IArrayBox const* ifab = &(imf[mfi]);
            AMREX_LAUNCH_HOST_DEVICE_LAMBDA (gbx, tbx,
            {
                amrex::cast(tbx, *lfab, *ifab, 0, 0, ncomp);
            });
        }

        return lmf;
    }

    std::unique_ptr<MultiFab> get_slice_data(int dir, Real coord, const MultiFab& cc, const Geometry& geom, int start_comp, int ncomp, bool interpolate) {

        BL_PROFILE("amrex::get_slice_data");

        if (interpolate) {
            AMREX_ASSERT(cc.nGrow() >= 1);
        }

        const auto geomdata = geom.data();

        Vector<int> slice_to_full_ba_map;
        std::unique_ptr<MultiFab> slice = allocateSlice(dir, cc, ncomp, geom, coord, slice_to_full_ba_map);

#ifdef _OPENMP
#pragma omp parallel if (Gpu::notInLaunchRegion())
#endif
        for (MFIter mfi(*slice, TilingIfNotGPU()); mfi.isValid(); ++mfi)
        {
            int slice_gid = mfi.index();
            int full_gid = slice_to_full_ba_map[slice_gid];
            FArrayBox* slice_fab = &((*slice)[mfi]);
            FArrayBox const* full_fab = &(cc[full_gid]);

            const Box& tile_box  = mfi.tilebox();

            if (interpolate)
            {
                AMREX_LAUNCH_HOST_DEVICE_LAMBDA ( tile_box, thread_box,
                {
                    amrex_fill_slice_interp(thread_box, *slice_fab, *full_fab,
                                            0, start_comp, ncomp,
                                            dir, coord, geomdata);
                });
            }
            else
            {
                AMREX_LAUNCH_HOST_DEVICE_LAMBDA ( tile_box, thread_box,
                {
                    slice_fab->copy(*full_fab, thread_box, start_comp, thread_box, 0, ncomp);
                });
            }
        }

        return slice;
    }

    iMultiFab makeFineMask (const MultiFab& cmf, const BoxArray& fba, const IntVect& ratio)
    {
        iMultiFab mask(cmf.boxArray(), cmf.DistributionMap(), 1, 0);
        const BoxArray& cfba = amrex::coarsen(fba,ratio);

#ifdef _OPENMP
#pragma omp parallel
#endif
        {
            std::vector< std::pair<int,Box> > isects;

            for (MFIter mfi(mask); mfi.isValid(); ++mfi)
            {
                IArrayBox& fab = mask[mfi];
                const Box& bx = fab.box();
                fab.setVal(0);
                cfba.intersections(bx, isects);
                for (auto const& is : isects)
                {
                    fab.setVal(1, is.second, 0, 1);
                }
            }
        }

        return mask;
    }

    void computeDivergence (MultiFab& divu, const Array<MultiFab const*,AMREX_SPACEDIM>& umac,
                            const Geometry& geom)
    {
        const Real* dxinv = geom.InvCellSize();
#ifdef _OPENMP
#pragma omp parallel
#endif
        for (MFIter mfi(divu,true); mfi.isValid(); ++mfi)
        {
            const Box& bx = mfi.tilebox();
            amrex_compute_divergence (BL_TO_FORTRAN_BOX(bx),
                                      BL_TO_FORTRAN_ANYD(divu[mfi]),
                                      AMREX_D_DECL(BL_TO_FORTRAN_ANYD((*umac[0])[mfi]),
                                                   BL_TO_FORTRAN_ANYD((*umac[1])[mfi]),
                                                   BL_TO_FORTRAN_ANYD((*umac[2])[mfi])),
                                      dxinv);
        }
    }
}
