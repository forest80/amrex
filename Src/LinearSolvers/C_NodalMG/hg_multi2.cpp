
#include "hg_multi.H"

#include <Profiler.H>

#if defined( BL_FORT_USE_UNDERSCORE )
#define   FORT_HGFRES		hgfres_
#define   FORT_HGFRES_TERRAIN   hgfres_terrain_
#define   FORT_HGFRES_FULL	hgfres_full_
#define   FORT_HGERES		hgeres_
#define   FORT_HGERES_TERRAIN   hgeres_terrain_
#define   FORT_HGERES_FULL	hgeres_full_
#define   FORT_HGCRES		hgcres_
#define   FORT_HGCRES_TERRAIN   hgcres_terrain_
#define   FORT_HGCRES_FULL	hgcres_full_
#elif defined( BL_FORT_USE_UPPERCASE )
#define   FORT_HGFRES		HGFRES
#define   FORT_HGFRES_TERRAIN   HGFRES_TERRAIN
#define   FORT_HGFRES_FULL	HGFRES_FULL
#define   FORT_HGERES		HGERES
#define   FORT_HGERES_TERRAIN   HGERES_TERRAIN
#define   FORT_HGERES_FULL	HGERES_FULL
#define   FORT_HGCRES		HGCRES
#define   FORT_HGCRES_TERRAIN   HGCRES_TERRAIN
#define   FORT_HGCRES_FULL	HGCRES_FULL
#elif defined( BL_FORT_USE_LOWERCASE )
#define   FORT_HGFRES		hgfres
#define   FORT_HGFRES_TERRAIN   hgfres_terrain
#define   FORT_HGFRES_FULL	hgfres_full
#define   FORT_HGERES		hgeres
#define   FORT_HGERES_TERRAIN   hgeres_terrain
#define   FORT_HGERES_FULL	hgeres_full
#define   FORT_HGCRES		hgcres
#define   FORT_HGCRES_TERRAIN   hgcres_terrain
#define   FORT_HGCRES_FULL	hgcres_full
#else
#error "none of BL_FORT_USE_{UNDERSCORE,UPPERCASE,LOWERCASE} defined"
#endif

extern "C"
{

#if (BL_SPACEDIM == 1)
#error not relevant
#endif
    void FORT_HGFRES        (Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS, intS,
			     CRealPS, intRS,
			     const int*, const int*, const int*);
    void FORT_HGFRES_FULL   (Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS, intS,
			     CRealPS, intRS,
			     const int*, const int*, const int*);
    void FORT_HGFRES_TERRAIN(Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS, intS,
			     CRealPS, intRS,
			     const int*, const int*, const int*);

    void FORT_HGCRES        (Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS, intS,
			     CRealPS, intRS, const int*, const int*);
    void FORT_HGCRES_FULL   (Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS, intS,
			     CRealPS, intRS, const int*, const int*);
    void FORT_HGCRES_TERRAIN(Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS, intS,
			     CRealPS, intRS, const int*, const int*);

    void FORT_HGERES        (Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS, intS,
			     CRealPS, intRS, const int*, const int*);
    void FORT_HGERES_FULL   (Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS, intS,
			     CRealPS, intRS, const int*, const int*);
    void FORT_HGERES_TERRAIN(Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS,
			     const Real*, intS, const Real*, intS, intS,
			     CRealPS, intRS, const int*, const int*);
}

extern "C"
{
typedef void (*F_FRES) (Real*, intS, const Real*, intS,
			const Real*, intS, const Real*, intS,
			const Real*, intS, const Real*, intS, intS,
			CRealPS, intRS,
			const int*, const int*, const int*);

typedef void (*F_CRES) (Real*, intS, const Real*, intS,
			const Real*, intS, const Real*, intS,
			const Real*, intS, const Real*, intS, intS,
			CRealPS, intRS,
			const int*, const int*);

typedef void (*F_ERES) (Real*, intS, const Real*, intS,
			const Real*, intS, const Real*, intS,
			const Real*, intS, const Real*, intS, intS,
			CRealPS, intRS,
			const int*, const int*);
}

class task_fres
    :
    public task_fec_base
{
public:

    task_fres (F_FRES          f_,
	       task_list&      tl_,
	       MultiFab&       r_,
	       const MultiFab& s_,
	       const MultiFab& d_,
	       const MultiFab& sg_,
	       int             igrid_,
	       task_fab*       c_,
	       task_fab*       sc_,
	       const Box&      creg_,
	       const Real      h_[BL_SPACEDIM],
	       const IntVect&  rat_,
	       int             idim_,
	       int             idir_,
	       int             isrz,
               char*           did_work);

    virtual bool ready ();

private:

    void doit ();

    F_FRES          f;
    const MultiFab& s;
    const MultiFab& d;
    const MultiFab& sg;
    const Box       creg;
    Real            h[BL_SPACEDIM];
    const IntVect   rat;
    const int       idim;
    const int       idir;
    const int       isrz;
};

task_fres::task_fres (F_FRES          f_,
		      task_list&      tl_,
		      MultiFab&       r_,
		      const MultiFab& s_,
		      const MultiFab& d_,
		      const MultiFab& sg_,
		      int             igrid_,
		      task_fab*       c_,
		      task_fab*       sc_,
		      const Box&      creg_,
		      const Real      h_[BL_SPACEDIM],
		      const IntVect&  rat_,
		      int             idim_,
		      int             idir_,
		      int             isrz_,
                      char*           did_work)
    :
    task_fec_base(tl_, r_, igrid_,did_work),
    f(f_),
    s(s_),
    d(d_),
    sg(sg_),
    creg(creg_),
    rat(rat_),
    idim(idim_),
    idir(idir_),
    isrz(isrz_)
{
    push_back(c_);
    push_back(sc_);

    D_TERM(h[0] = h_[0];, h[1] = h_[1];, h[2] = h_[2];);

    if (is_local_target() && dependencies.empty()) doit();
}

bool
task_fres::ready ()
{
  //    BL_PROFILE(BL_PROFILE_THIS_NAME() + "::ready()");

    BL_ASSERT(!m_finished);

    if (is_local_target()) doit();

    return true;
}

void
task_fres::doit ()
{
    BL_ASSERT(!m_finished);
    BL_ASSERT(is_local_target());
    BL_ASSERT(dependencies.empty());

    if (m_did_work) *m_did_work = 1;

    m_finished = true;

    const int        igrid      = grid_number();
    FArrayBox&       r_fab      = target_fab();
    const Box&       r_fab_box  = r_fab.box();
    const FArrayBox& s_fab      = s[igrid];
    const Box&       s_fab_box  = s_fab.box();
    const FArrayBox& d_fab      = d[igrid];
    const Box&       d_fab_box  = d_fab.box();
    const FArrayBox& c_fab      = task_fab_result(0);
    const Box&       c_fab_box  = c_fab.box();
    const FArrayBox& sg_fab     = sg[igrid];
    const Box&       sg_fab_box = sg_fab.box();
    const FArrayBox& sc_fab     = task_fab_result(1);
    const Box&       sc_fab_box = sc_fab.box();

    (*f)(r_fab.dataPtr(), DIMLIST(r_fab_box),
	 s_fab.dataPtr(), DIMLIST(s_fab_box),
	 d_fab.dataPtr(), DIMLIST(d_fab_box),
	 c_fab.dataPtr(), DIMLIST(c_fab_box),
	 sg_fab.dataPtr(), DIMLIST(sg_fab_box),
	 sc_fab.dataPtr(), DIMLIST(sc_fab_box),
	 DIMLIST(creg),
	 D_DECL(&h[0], &h[1], &h[2]),
	 D_DECL(rat[0], rat[1], rat[2]), &idim, &idir, &isrz);
}


class task_eres
    :
    public task_fec_base
{
public:

    task_eres (F_ERES            f_,
	       task_list&        tl_,
	       MultiFab&         r_,
	       const MultiFab&   s_,
	       int               igrid_,
	       task_fab*         ff_,
	       task_fab*         cc_,
	       task_fab*         sigmaf_,
	       task_fab*         sigmac_,
	       const Box&        creg_,
	       const Real        h_[BL_SPACEDIM],
	       const IntVect&    rat_,
	       const Array<int>& ga_,
	       const IntVect&    t_,
               char*             did_work);

    virtual bool ready ();

private:

    void doit ();

    const F_ERES    f;
    const MultiFab& s;
    const Box       creg;
    Real            h[BL_SPACEDIM];
    const IntVect   rat;
    Array<int>      ga;
    Array<int>      t;
};

task_eres::task_eres (F_ERES            f_,
		      task_list&        tl_,
		      MultiFab&         r_,
		      const MultiFab&   s_,
		      int               igrid_,
		      task_fab*         ff_,
		      task_fab*         cc_,
		      task_fab*         sigmaf_,
		      task_fab*         sigmac_,
		      const Box&        creg_,
		      const Real        h_[BL_SPACEDIM],
		      const IntVect&    rat_,
		      const Array<int>& ga_,
		      const IntVect&    t_,
                      char*             did_work)
    :
    task_fec_base(tl_, r_, igrid_,did_work),
    f(f_),
    s(s_),
    creg(creg_),
    rat(rat_),
    ga(ga_),
    t(BL_SPACEDIM)
{
    push_back(ff_);
    push_back(cc_);
    push_back(sigmaf_);
    push_back(sigmac_);

    D_TERM(h[0] = h_[0];, h[1] = h_[1];, h[2] = h_[2];);
    D_TERM(t[0] = t_[0];, t[1] = t_[1];, t[2] = t_[2];);

    if (is_local_target() && dependencies.empty()) doit();
}

bool
task_eres::ready ()
{
  //    BL_PROFILE(BL_PROFILE_THIS_NAME() + "::ready()");

    BL_ASSERT(!m_finished);

    if (is_local_target()) doit();

    return true;
}

void
task_eres::doit ()
{
    BL_ASSERT(!m_finished);
    BL_ASSERT(is_local_target());
    BL_ASSERT(dependencies.empty());

    if (m_did_work) *m_did_work = 1;

    m_finished = true;

    const int        igrid          = grid_number();
    FArrayBox&       r_fab          = target_fab();
    const Box&       r_fab_box      = r_fab.box();
    const FArrayBox& s_fab          = s[igrid];
    const Box&       s_fab_box      = s_fab.box();
    const FArrayBox& ff_fab         = task_fab_result(0);
    const Box&       ff_fab_box     = ff_fab.box();
    const FArrayBox& cc_fab         = task_fab_result(1);
    const Box&       cc_fab_box     = cc_fab.box();
    const FArrayBox& sigmaf_fab     = task_fab_result(2);
    const Box&       sigmaf_fab_box = sigmaf_fab.box();
    const FArrayBox& sigmac_fab     = task_fab_result(3);
    const Box&       sigmac_fab_box = sigmac_fab.box();

    (*f)(r_fab.dataPtr(), DIMLIST(r_fab_box),
	 s_fab.dataPtr(), DIMLIST(s_fab_box),
	 ff_fab.dataPtr(), DIMLIST(ff_fab_box),
	 cc_fab.dataPtr(), DIMLIST(cc_fab_box),
	 sigmaf_fab.dataPtr(), DIMLIST(sigmaf_fab_box),
	 sigmac_fab.dataPtr(), DIMLIST(sigmac_fab_box),
	 DIMLIST(creg),
	 D_DECL(&h[0], &h[1], &h[2]),
	 D_DECL(rat[0], rat[1], rat[2]), ga.dataPtr(), t.dataPtr());
}

class task_cres
    :
    public task_fec_base
{
public:

    task_cres (F_CRES           f_,
	       task_list&        tl_,
	       MultiFab&         r_,
	       const MultiFab&   s_,
	       int               igrid_,
	       task_fab*         ff_,
	       task_fab*         cc_,
	       task_fab*         sigmaf_,
	       task_fab*         sigmac_,
	       const Box&        creg_,
	       const Real        h_[BL_SPACEDIM],
	       const IntVect&    rat_,
	       const Array<int>& ga_,
	       const int         isrz_,
               char*             did_work);

    virtual bool ready ();

private:

    void doit ();

    F_CRES          f;
    const MultiFab& s;
    const Box       creg;
    Real            h[BL_SPACEDIM];
    const IntVect   rat;
    Array<int>      ga;
    const int       isrz;
};

task_cres::task_cres (F_CRES           f_,
		      task_list&        tl_,
		      MultiFab&         r_,
		      const MultiFab&   s_,
		      int               igrid_,
		      task_fab*         ff_,
		      task_fab*         cc_,
		      task_fab*         sigmaf_,
		      task_fab*         sigmac_,
		      const Box&        creg_,
		      const Real        h_[BL_SPACEDIM],
		      const IntVect&    rat_,
		      const Array<int>& ga_,
		      const int         isrz_,
                      char*             did_work)
    :
    task_fec_base(tl_, r_, igrid_,did_work),
    f(f_),
    s(s_),
    creg(creg_),
    rat(rat_),
    ga(ga_),
    isrz(isrz_)
{
    push_back(ff_);
    push_back(cc_);
    push_back(sigmaf_);
    push_back(sigmac_);

    D_TERM(h[0] = h_[0];, h[1] = h_[1];, h[2] = h_[2];);

    if (is_local_target() && dependencies.empty()) doit();
}

bool
task_cres::ready ()
{
  //    BL_PROFILE(BL_PROFILE_THIS_NAME() + "::ready()");

    BL_ASSERT(!m_finished);

    if (is_local_target()) doit();

    return true;
}

void
task_cres::doit ()
{
    BL_ASSERT(!m_finished);
    BL_ASSERT(is_local_target());
    BL_ASSERT(dependencies.empty());

    if (m_did_work) *m_did_work = 1;

    m_finished = true;

    const int        igrid          = grid_number();
    FArrayBox&       r_fab          = target_fab();
    const Box&       r_fab_box      = r_fab.box();
    const FArrayBox& s_fab          = s[igrid];
    const Box&       s_fab_box      = s_fab.box();
    const FArrayBox& ff_fab         = task_fab_result(0);
    const Box&       ff_fab_box     = ff_fab.box();
    const FArrayBox& cc_fab         = task_fab_result(1);
    const Box&       cc_fab_box     = cc_fab.box();
    const FArrayBox& sigmaf_fab     = task_fab_result(2);
    const Box&       sigmaf_fab_box = sigmaf_fab.box();
    const FArrayBox& sigmac_fab     = task_fab_result(3);
    const Box&       sigmac_fab_box = sigmac_fab.box();

    (*f)(r_fab.dataPtr(), DIMLIST(r_fab_box),
	 s_fab.dataPtr(), DIMLIST(s_fab_box),
	 ff_fab.dataPtr(), DIMLIST(ff_fab_box),
	 cc_fab.dataPtr(), DIMLIST(cc_fab_box),
	 sigmaf_fab.dataPtr(), DIMLIST(sigmaf_fab_box),
	 sigmac_fab.dataPtr(), DIMLIST(sigmac_fab_box),
	 DIMLIST(creg),
	 D_DECL(&h[0], &h[1], &h[2]),
	 D_DECL(rat[0], rat[1], rat[2]), ga.dataPtr(), &isrz);
}

void
holy_grail_amr_multigrid::alloc_sync_caches ()
{
    if (lev_min < lev_max)
    {
	fres_fbox  .resize(lev_max+1);
	fres_cbox  .resize(lev_max+1);
	fres_creg  .resize(lev_max+1);
	fres_sfbox .resize(lev_max+1);
	fres_scbox .resize(lev_max+1);
#if (BL_SPACEDIM == 3)
	eres_fbox  .resize(lev_max+1);
	eres_cbox  .resize(lev_max+1);
	eres_creg  .resize(lev_max+1);
	eres_sfbox .resize(lev_max+1);
	eres_scbox .resize(lev_max+1);
#endif
	cres_fbox  .resize(lev_max+1);
	cres_cbox  .resize(lev_max+1);
	cres_creg  .resize(lev_max+1);
	cres_sfbox .resize(lev_max+1);
	cres_scbox .resize(lev_max+1);
    }

    // This loop kill gcc 2.95.2 under some situations.
    for (int lev(lev_min + 1); lev <= lev_max; ++lev)
    {
	fres_sfbox[lev] .resize(lev_interface[ml_index[lev]].nboxes(level_interface::FACEDIM));
	fres_scbox[lev] .resize(lev_interface[ml_index[lev]].nboxes(level_interface::FACEDIM));
	fres_fbox[lev]  .resize(lev_interface[ml_index[lev]].nboxes(level_interface::FACEDIM));
	fres_cbox[lev]  .resize(lev_interface[ml_index[lev]].nboxes(level_interface::FACEDIM));
	fres_creg[lev]  .resize(lev_interface[ml_index[lev]].nboxes(level_interface::FACEDIM));

#if (BL_SPACEDIM == 3)
	eres_sfbox[lev] .resize(lev_interface[ml_index[lev]].nboxes(1));
	eres_scbox[lev] .resize(lev_interface[ml_index[lev]].nboxes(1));
	eres_fbox[lev]  .resize(lev_interface[ml_index[lev]].nboxes(1));
	eres_cbox[lev]  .resize(lev_interface[ml_index[lev]].nboxes(1));
	eres_creg[lev]  .resize(lev_interface[ml_index[lev]].nboxes(1));
#endif
	
	cres_sfbox[lev] .resize(lev_interface[ml_index[lev]].nboxes(0));
	cres_scbox[lev] .resize(lev_interface[ml_index[lev]].nboxes(0));
	cres_fbox[lev]  .resize(lev_interface[ml_index[lev]].nboxes(0));
	cres_cbox[lev]  .resize(lev_interface[ml_index[lev]].nboxes(0));
	cres_creg[lev]  .resize(lev_interface[ml_index[lev]].nboxes(0));
	
	build_sync_cache(ml_index[lev], lev);
    }
}

void
holy_grail_amr_multigrid::delete_sync_caches ()
{
    for (int lev = lev_min + 1; lev <= lev_max; ++lev)
    {
	fres_fbox[lev].clear();
	fres_cbox[lev].clear();
	fres_creg[lev].clear();
	fres_sfbox[lev].clear();
	fres_scbox[lev].clear();
#if (BL_SPACEDIM == 3)
	eres_fbox[lev].clear();
	eres_cbox[lev].clear();
	eres_creg[lev].clear();
	eres_sfbox[lev].clear();
	eres_scbox[lev].clear();
#endif
	cres_fbox[lev].clear();
	cres_cbox[lev].clear();
	cres_creg[lev].clear();
	cres_sfbox[lev].clear();
	cres_scbox[lev].clear();
    }
    if (lev_min < lev_max)
    {
	fres_fbox.clear();
	fres_cbox.clear();
	fres_creg.clear();
	fres_sfbox.clear();
	fres_scbox.clear();
#if (BL_SPACEDIM == 3)
	eres_fbox.clear();
	eres_cbox.clear();
	eres_creg.clear();
	eres_sfbox.clear();
	eres_scbox.clear();
#endif
	cres_fbox.clear();
	cres_cbox.clear();
	cres_creg.clear();
	cres_sfbox.clear();
	cres_scbox.clear();
    }
}

void
holy_grail_amr_multigrid::build_sync_cache (int mglev,
                                            int lev)
{
    const IntVect& rat    = gen_ratio[lev-1];
    const int      mglevc = ml_index[lev-1];

    for (int iface = 0;
	 iface < lev_interface[mglev].nboxes(level_interface::FACEDIM);
	 iface++)
    {
        //
	// Find a fine grid touching this face.
        //
	int igrid =
	    lev_interface[mglev].grid(level_interface::FACEDIM, iface, 0);
	if (igrid < 0)
	    igrid = lev_interface[mglev].grid(level_interface::FACEDIM,
					      iface, 1);
	const unsigned int geo =
	    lev_interface[mglev].geo(level_interface::FACEDIM, iface);
        //
	// Reject fine-fine interfaces and those without an interior fine grid
        //
	if (   geo == level_interface::ALL
	    || igrid < 0
	    || lev_interface[mglev].flag(level_interface::FACEDIM, iface) )
	{
	    continue;
	}
        //
	// Fine grid on just one side.
        //
	const int idim = lev_interface[mglev].fdim(iface);
	const int idir = (geo & level_interface::LOW) ? -1 : 1;
	Box& fbox = fres_fbox[lev][iface];
	Box& cbox = fres_cbox[lev][iface];
	Box& creg = fres_creg[lev][iface];
	fbox = dest[lev].box(igrid); fbox.grow(dest[lev].nGrow());
	BL_ASSERT(is_remote(dest[lev], igrid)
		  || fbox == dest[lev][igrid].box());
	// fbox = dest[lev][igrid].box();
	cbox = lev_interface[mglev].node_box(level_interface::FACEDIM, iface);
	cbox.coarsen(rat);
	if (idir > 0)
	    cbox.growLo(idim, 1);
	else
	    cbox.growHi(idim, 1);
	Box& sigmafbox = fres_sfbox[lev][iface];
	Box& sigmacbox = fres_scbox[lev][iface];
	sigmafbox = sigma[mglev].box(igrid);
	sigmafbox.grow(sigma[mglev].nGrow());
	BL_ASSERT(    is_remote(sigma[mglev], igrid)
		   || sigmafbox == sigma[mglev][igrid].box());
	// sigmafbox = sigma[mglev][igrid].box();
	sigmacbox = cbox;
	sigmacbox.convert(IntVect::TheCellVector());
	const IntVect t =
	    lev_interface[mglev].box(level_interface::FACEDIM, iface).type();
	creg = lev_interface[mglev].node_box(level_interface::FACEDIM, iface);
	creg.coarsen(rat).grow(t - 1);
    }

#if (BL_SPACEDIM == 3)
    for (int iedge = 0; iedge < lev_interface[mglev].nboxes(1); iedge++)
    {
        //
	// Find a fine grid touching this edge.
        //
	int igrid;
	for (int i = 0; i < lev_interface[mglev].ngrids(1); i++)
	{
	    igrid = lev_interface[mglev].grid(1, iedge, i);
	    if (igrid >= 0)
		break;
	}
	const unsigned int geo = lev_interface[mglev].geo(1, iedge);
        //
	// Reject fine-fine interfaces and those without an interior fine grid
        //
	if ( geo == level_interface::ALL
	     || igrid < 0
	     || lev_interface[mglev].flag(1, iedge) )
	{
	    continue;
	}
	Box& fbox = eres_fbox[lev][iedge];
	Box& cbox = eres_cbox[lev][iedge];
	Box& creg = eres_creg[lev][iedge];
	const IntVect t = lev_interface[mglev].box(1, iedge).type();
	cbox = lev_interface[mglev].node_box(1, iedge);
	cbox.coarsen(rat).grow(t);
	fbox = BoxLib::refine(cbox, rat);
	Box& sigmafbox = eres_sfbox[lev][iedge];
	Box& sigmacbox = eres_scbox[lev][iedge];
	sigmafbox = fbox;
	sigmafbox.convert(IntVect::TheCellVector());
	sigmacbox = cbox;
	sigmacbox.convert(IntVect::TheCellVector());
	creg = lev_interface[mglev].node_box(1, iedge);
	creg.coarsen(rat).grow(t - 1);
    }

#endif
    for (int icor = 0; icor < lev_interface[mglev].nboxes(0); icor++)
    {
        //
	// Find a fine grid touching this corner.
        //
	int igrid;
	for (int i = 0; i < lev_interface[mglev].ngrids(0); i++)
	{
	    igrid = lev_interface[mglev].grid(0, icor, i);
	    if (igrid >= 0)
		break;
	}
	const unsigned int geo = lev_interface[mglev].geo(0, icor);
        //
	// Reject fine-fine interfaces and those without an interior fine grid
        //
	if (geo == level_interface::ALL
	    || igrid < 0
	    || lev_interface[mglev].flag(0, icor) )
	{
	    continue;
	}
	Box& fbox = cres_fbox[lev][icor];
	Box& cbox = cres_cbox[lev][icor];
	Box& creg = cres_creg[lev][icor];
	cbox = lev_interface[mglev].box(0, icor);
	fbox = lev_interface[mglev].box(0, icor);
	cbox.coarsen(rat).grow(1);
	fbox.grow(rat);
	Box& sigmafbox = cres_sfbox[lev][icor];
	Box& sigmacbox = cres_scbox[lev][icor];
	sigmafbox = fbox;
	sigmafbox.convert(IntVect::TheCellVector());
	sigmacbox = cbox;
	sigmacbox.convert(IntVect::TheCellVector());
	creg = lev_interface[mglev].box(0, icor);
	creg.coarsen(rat);
    }
}

void
holy_grail_amr_multigrid::interface_residual (int mglev,
                                              int lev)
{
    BL_PROFILE(BL_PROFILE_THIS_NAME() + "::interface_residual()");

    // FIXME boundary not terrain sigma?
    const amr_boundary* bndry =	(m_stencil==terrain) ? boundary.terrain_sigma() : boundary.scalar();

    const IntVect& rat    = gen_ratio[lev-1];
    const int      mglevc = ml_index[lev-1];

    std::vector<char>& irv_f = lev_interface[mglev].m_interface_residual_f[std::pair<int,int>(mglev,lev)];
    std::vector<char>& irv_e = lev_interface[mglev].m_interface_residual_e[std::pair<int,int>(mglev,lev)];
    std::vector<char>& irv_c = lev_interface[mglev].m_interface_residual_c[std::pair<int,int>(mglev,lev)];

    if (irv_f.size() != lev_interface[mglev].nboxes(level_interface::FACEDIM))
    {
        irv_f.resize(lev_interface[mglev].nboxes(level_interface::FACEDIM));
        for (int i = 0; i < irv_f.size(); i++) irv_f[i] = 1;
    }
    if (irv_e.size() != lev_interface[mglev].nboxes(1))
    {
        irv_e.resize(lev_interface[mglev].nboxes(1));
        for (int i = 0; i < irv_e.size(); i++) irv_e[i] = 1;
    }
    if (irv_c.size() != lev_interface[mglev].nboxes(0))
    {
        irv_c.resize(lev_interface[mglev].nboxes(0));
        for (int i = 0; i < irv_c.size(); i++) irv_c[i] = 1;
    }
    
    HG_TEST_NORM(resid[mglev], "interface_residual");

    task_list tl;

    for (int iface = 0;
	 iface < lev_interface[mglev].nboxes(level_interface::FACEDIM);
	 iface++)
    {
        if (irv_f[iface])
        {
            char* did_work = &irv_f[iface];

            *did_work = 0;
            //
            // Find a fine grid touching this face.
            //
            int igrid = lev_interface[mglev].grid(level_interface::FACEDIM, iface, 0);
            if (igrid < 0)
                igrid = lev_interface[mglev].grid(level_interface::FACEDIM, iface, 1);
            const unsigned int geo = lev_interface[mglev].geo(level_interface::FACEDIM, iface);
            //
            // Reject fine-fine interfaces and those without an interior fine grid
            //
            if (geo == level_interface::ALL
                || igrid < 0
                || lev_interface[mglev].flag(level_interface::FACEDIM, iface) )
                continue;
            //
            // Fine grid on just one side.
            //
            const int  idim      = lev_interface[mglev].fdim(iface);
            const int  idir      = (geo & level_interface::LOW) ? -1 : 1;
            const Box& cbox      = fres_cbox[lev][iface];
            const Box& sigmacbox = fres_scbox[lev][iface];
            task_fab* sigmac = new task_fill_patch(tl, resid[mglev], igrid, sigmacbox,
                                                   sigma[mglevc],
                                                   lev_interface[mglevc],
                                                   bndry, -1, -1, did_work);
            task_fab* cdst = new task_fill_patch(tl, resid[mglev], igrid, cbox,
                                                 dest[lev-1],
                                                 lev_interface[mglevc],
                                                 boundary.pressure(), -1, -1, did_work);
            const Box& creg = fres_creg[lev][iface];
            if (m_stencil == terrain)
            {
                tl.add_task(new task_fres(&FORT_HGFRES_TERRAIN, tl, resid[mglev],
                                          source[lev], dest[lev], sigma[mglev],
                                          igrid, cdst, sigmac, creg,
                                          h[mglev], rat, idim, idir, 0, did_work));
            }
            else if (m_stencil == full)
            {
                tl.add_task(new task_fres(&FORT_HGFRES_FULL, tl, resid[mglev],
                                          source[lev], dest[lev], sigma[mglev],
                                          igrid, cdst, sigmac, creg,
                                          h[mglev], rat, idim, idir, 0, did_work));
            }
            else
            {
                const int isRZ = getCoordSys();
                tl.add_task(new task_fres(&FORT_HGFRES, tl, resid[mglev],
                                          source[lev], dest[lev], sigma[mglev],
                                          igrid, cdst, sigmac, creg,
                                          h[mglev], rat, idim, idir, isRZ, did_work));
            }
        }
    }
    tl.execute("holy_grail_amr_multigrid::interface_residual(1)");

    HG_TEST_NORM( resid[mglev], "interface_residual a");

#if (BL_SPACEDIM == 3)
    for (int iedge = 0; iedge < lev_interface[mglev].nboxes(1); iedge++)
    {
        if (irv_e[iedge])
        {
            char* did_work = &irv_e[iedge];

            *did_work = 0;
            //
            // Find a fine grid touching this edge.
            //
            int igrid;
            for (int i = 0; i < lev_interface[mglev].ngrids(1); i++)
            {
                igrid = lev_interface[mglev].grid(1, iedge, i);
                if (igrid >= 0)
                    break;
            }
            const unsigned int geo = lev_interface[mglev].geo(1, iedge);
            //
            // Reject fine-fine interfaces and those without an interior fine grid
            //
            if (geo != level_interface::ALL
                && igrid >= 0
                && !lev_interface[mglev].flag(1, iedge) )
            {
                const Box& fbox      = eres_fbox[lev][iedge];
                const Box& cbox      = eres_cbox[lev][iedge];
                const Box& sigmafbox = eres_sfbox[lev][iedge];
                const Box& sigmacbox = eres_scbox[lev][iedge];
                task_fab* sigmaf = new task_fill_patch(tl,
                                                       resid[mglev], igrid, sigmafbox,
                                                       sigma[mglev],
                                                       lev_interface[mglev],
                                                       bndry, 1, iedge, did_work);
                task_fab* sigmac = new task_fill_patch(tl,
                                                       resid[mglev], igrid, sigmacbox,
                                                       sigma[mglevc],
                                                       lev_interface[mglevc],
                                                       bndry, -1, -1, did_work);
                const Box& creg = eres_creg[lev][iedge];
                const IntVect t = lev_interface[mglev].box(1, iedge).type();
                task_fab* fdst = new task_fill_patch(tl, resid[mglev], igrid, fbox,
                                                     dest[lev],
                                                     lev_interface[mglev],
                                                     boundary.pressure(), 1, iedge, did_work);
                task_fab* cdst = new task_fill_patch(tl,
                                                     resid[mglev], igrid, cbox,
                                                     dest[lev-1],
                                                     lev_interface[mglevc],
                                                     boundary.pressure(), -1, -1, did_work);
                Array<int> ga = lev_interface[mglev].geo_array(1, iedge);
                task::task_proxy tp;
                if (m_stencil == terrain)
                {
                    tp = tl.add_task(new task_eres(&FORT_HGERES_TERRAIN, tl, resid[mglev],
                                                   source[lev], igrid, fdst, cdst,
                                                   sigmaf, sigmac, creg,
                                                   h[mglev], rat, ga, t, did_work));
                }
                else if (m_stencil == full)
                {
                    tp = tl.add_task(new task_eres(&FORT_HGERES_FULL, tl, resid[mglev],
                                                   source[lev], igrid, fdst, cdst,
                                                   sigmaf, sigmac, creg,
                                                   h[mglev], rat, ga, t, did_work));
                }
                else
                {
                    tp = tl.add_task(new task_eres(&FORT_HGERES, tl, resid[mglev],
                                                   source[lev], igrid, fdst, cdst,
                                                   sigmaf, sigmac, creg,
                                                   h[mglev], rat, ga, t, did_work));
                }
                //
                // Fill in the grids on the other sides, if any.
                //
                const Box& freg = lev_interface[mglev].node_box(1, iedge);
                for (int i = 1; i < lev_interface[mglev].ngrids(1); i++)
                {
                    const int jgrid = lev_interface[mglev].grid(1, iedge, i);
                    if (jgrid >= 0 && jgrid != igrid)
                    {
                        tl.add_task(new task_copy(tl, resid[mglev], jgrid,
                                                  resid[mglev], igrid, freg, tp, did_work));
                    }
                }
            }
        }
    }
    tl.execute("holy_grail_amr_multigrid::interface_residual(2)");

    HG_TEST_NORM( resid[mglev], "interface_residual b");
#endif
    for (int icor = 0; icor < lev_interface[mglev].nboxes(0); icor++)
    {
        if (irv_c[icor])
        {
            char* did_work = &irv_c[icor];

            *did_work = 0;
            //
            // Find a fine grid touching this corner.
            //
            int igrid;
            for (int i = 0; i < lev_interface[mglev].ngrids(0); i++)
            {
                igrid = lev_interface[mglev].grid(0, icor, i);
                if (igrid >= 0)
                    break;
            }
            const unsigned int geo = lev_interface[mglev].geo(0, icor);
            //
            // Reject fine-fine interfaces and those without an interior fine grid
            //
            if (geo != level_interface::ALL
                && igrid >= 0
                && !lev_interface[mglev].flag(0, icor) )
            {
                const Box& fbox      = cres_fbox[lev][icor];
                const Box& cbox      = cres_cbox[lev][icor];
                const Box& sigmafbox = cres_sfbox[lev][icor];
                const Box& sigmacbox = cres_scbox[lev][icor];
                task_fab* sigmaf = new task_fill_patch(tl, resid[mglev], igrid, sigmafbox,
                                                       sigma[mglev],
                                                       lev_interface[mglev],
                                                       bndry, 0, icor, did_work);
                task_fab* sigmac = new task_fill_patch(tl, resid[mglev], igrid, sigmacbox,
                                                       sigma[mglevc],
                                                       lev_interface[mglevc],
                                                       bndry, -1, -1, did_work);
                const Box& creg = cres_creg[lev][icor];
                task_fab* fdst = new task_fill_patch(tl, resid[mglev], igrid, fbox,
                                                     dest[lev],
                                                     lev_interface[mglev],
                                                     boundary.pressure(), 0, icor, did_work);
                task_fab* cdst = new task_fill_patch(tl, resid[mglev], igrid, cbox,
                                                     dest[lev-1],
                                                     lev_interface[mglevc],
                                                     boundary.pressure(), -1, -1, did_work);
                Array<int> ga = lev_interface[mglev].geo_array(0, icor);
                task::task_proxy tp;
                if (m_stencil == terrain)
                {
                    tp = tl.add_task(new task_cres(&FORT_HGCRES_TERRAIN, tl, resid[mglev],
                                                   source[lev], igrid, fdst, cdst,
                                                   sigmaf, sigmac, creg,
                                                   h[mglev], rat, ga, 0, did_work));
                }
                else if (m_stencil == full)
                {
                    tp = tl.add_task(new task_cres(&FORT_HGCRES_FULL, tl, resid[mglev],
                                                   source[lev], igrid, fdst, cdst,
                                                   sigmaf, sigmac, creg,
                                                   h[mglev], rat, ga, 0, did_work));
                }
                else
                {
                    const int isrz = getCoordSys();
                    tp = tl.add_task(new task_cres(&FORT_HGCRES, tl, resid[mglev],
                                                   source[lev], igrid, fdst, cdst,
                                                   sigmaf, sigmac, creg,
                                                   h[mglev], rat, ga, isrz, did_work));
                }
                //
                // Fill in the grids on the other sides, if any.
                //
                const Box& freg = lev_interface[mglev].box(0, icor);
                for (int i = 1; i < lev_interface[mglev].ngrids(0); i++)
                {
                    const int jgrid = lev_interface[mglev].grid(0, icor, i);
                    if (jgrid >= 0 && jgrid != igrid)
                    {
                        tl.add_task(new task_copy(tl, resid[mglev], jgrid,
                                                  resid[mglev], igrid, freg, tp, did_work));
                    }
                }
            }
        }
    }
    tl.execute("holy_grail_amr_multigrid::interface_residual(3)");

    if (ParallelDescriptor::IOProcessor() && false)
    {
        int sum_f = 0, sum_e = 0, sum_c = 0;

        for (int i = 0; i < irv_f.size(); i++) if (irv_f[i]) sum_f++;
        for (int i = 0; i < irv_e.size(); i++) if (irv_e[i]) sum_e++;
        for (int i = 0; i < irv_c.size(); i++) if (irv_c[i]) sum_c++;
        
        std::cout << "m_interface_residual_f: used " << sum_f << " out of " << irv_f.size() << "\n";
        std::cout << "m_interface_residual_e: used " << sum_e << " out of " << irv_e.size() << "\n";
        std::cout << "m_interface_residual_c: used " << sum_c << " out of " << irv_c.size() << "\n";
    }

    HG_TEST_NORM(resid[mglev], "interface_residual c");
    HG_TEST_NORM(resid[mglev], "interface_residual end");
}
