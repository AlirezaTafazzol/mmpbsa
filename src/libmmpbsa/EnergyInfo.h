/**
 * @class mmpbsa::EnergyInfo
 * @brief Molecular dynamics wrapper
 *
 *
 * Created by David Coss <David.Coss@stjude.org> 2010
 */

#ifndef ENERGYINFO_H
#define	ENERGYINFO_H


#include "SanderParm.h"
#include "globals.h"
#include <valarray>
#include <string>

#define MDOUT_AVERAGE_HEADER "A V E R A G E"
#define CONVERGENCE_STRING "Maximum number of minimization cycles reached."


namespace mmpbsa{

class EnergyInfo : public std::valarray<mmpbsa_t> {
public:
    enum CONVERGENCE_VALUES{UNKNOWN = -1, NOT_CONVERGED = 0, CONVERGED = 1};

    /**
     * MD Energy Class.
     * Wraps the energy data, providing arithmetic over the set of energy data.
     * 
     * By default, minimization is set to false, i.e. assumed to be MD unless
     * determined otherwise.
     */
  EnergyInfo() : std::valarray<mmpbsa_t>(0.0,total_parameters){is_minimization = false;has_converged = UNKNOWN;}

  EnergyInfo(const EnergyInfo& rhs) : std::valarray<mmpbsa_t>(rhs){is_minimization = rhs.is_minimization;has_converged = rhs.has_converged;}
    virtual ~EnergyInfo(){}

    //EnergyInfo& operator=(const EnergyInfo& rhs){std::valarray<mmpbsa_t>::operator=(rhs);is_minimization = rhs.is_minimization;return *this;}

    /**
     * Loads first energy info from the provided mdout file.
     * 
     * Returns an integer equal to zero if successful. Otherwise values correspond to mmpbsa::MMPBSAErrorTypes
     *
     * @param fileName
     * @return error int equal to zero if successful. Otherwise values correspond to mmpbsa::MMPBSAErrorTypes.
     */
    int get_first_energyinfo(const char* fileName);
    int get_first_energyinfo(const std::string& filename){return get_first_energyinfo(filename.c_str());}

    /**
     * Loads next energy info from the provided mdout file.
     * 
     * Returns an integer equal to zero if successful. Otherwise values correspond to mmpbsa::MMPBSAErrorTypes
     *
     * @param mdoutFile
     * @return error int equal to zero if successful. Otherwise values correspond to mmpbsa::MMPBSAErrorTypes.
     */
    int get_next_energyinfo(std::istream& mdoutFile);

    /**
     * Stores the provided energy value in the correct place in the EnergyInfo class
     * Returns true if the identifier is a correct energy type. Returns false otherwise.
     * 
     * @param identifier
     * @param value
     * @return
     */
    bool loadEnergyValue(const std::string& identifier,const mmpbsa_t& value);
    bool loadEnergyValue(const std::string& identifier,const std::string& value);

    /**
     * Gives the NSTEP, Etotal, RMS values for a minimization NSTEP as a
     * pointer to a mmpbsa_t[2]. If the values are incorrect or absent,
     * an SanderIOException is thrown.
     */
    static mmpbsa_t* get_minimization_header(const std::string& header_line) throw (mmpbsa::SanderIOException);

    
    void clear();

    /**
     * Returns true if at least one element is greater than the corresponding
     * element of rhs. Otherwise, false is returned.
     * 
     * @param rhs
     * @return
     */
    bool operator>(const EnergyInfo& rhs)const;


    /**
     * energy information from sander/dynlib.f:prntmd
     *
     */
    enum EnergyInfoIndices {
        nstep, /*step number*/
        time,
        temp,
        press,
        etot,
        ektot,
        eptot,
        bond,
        angle,
        dihed,
        nb14,
        eel14,
        vdwaals,
        eelec,
        ehbond,
        egb,
        restraint,
        esurf,
        nonconst_pot,
        dvdl,
        rms_pbs,
        ekcmt,
        virial,
        volume,
        eksolt,
        eksolv,
        epol,
        e3bod,
        diprms,
        dipitr,
        dipole_temp,
        density,
        ewalderr,
        rmsdvalue,
        tgtrmsd,
        total_parameters/*gives the total number of energy data types in the EnergyInfo class. For use with loops internally.*/
    };

    bool is_minimization;///< Indicates whether or not this is a minimization energy set. Default: FALSE
    
    int  has_converged;///< Indicates whether or not the convergence flag has been seen. Default: UNKNOWN
    
};

class AveRmsEnerInfo
{
public:
    AveRmsEnerInfo();
    AveRmsEnerInfo(const AveRmsEnerInfo& orig);
    AveRmsEnerInfo(const EnergyInfo& avgs, const EnergyInfo& rmses);

    virtual ~AveRmsEnerInfo(){}

    /**
     * Loads next energy info from the provided mdout file.
     *
     * @param mdoutFile
     * @return
     */
    int get_avg_rms_info(std::istream& mdoutFile);

    void clear();

    AveRmsEnerInfo& operator=(const AveRmsEnerInfo& rhs);
    
    const EnergyInfo& get_average()const;
    const EnergyInfo& get_rms()const;
    const EnergyInfo& get_relrms()const;

 protected:
    EnergyInfo avg;///<Averages
    EnergyInfo rms;///<Root Mean Squares
    EnergyInfo relrms;//rms/abs(avg)

};

/**
 * Return final energy from the mdout file of a minization run
 * 
 * @param mdout
 * @return
 */
mmpbsa_t get_minimized_energy(std::istream& mdout) throw (SanderIOException);

/**
 * Takes an index into EnergyInfo class and returns the string representation of that
 * energy type according to the Sander mdout file format.
 *
 * If energy_type is not a valid index of EnergyInfo, "UNKNOWN" is returned instead.
 */
std::string str_energy_type(int energy_type);

};//end namespace mmpbsa

mmpbsa::EnergyInfo sqrt(const mmpbsa::EnergyInfo& rhs);
mmpbsa::EnergyInfo abs(const mmpbsa::EnergyInfo& rhs);
std::ostream& operator<<(std::ostream& theStream, const mmpbsa::EnergyInfo& data);
std::istream& operator>>(std::istream& theStream, mmpbsa::EnergyInfo& data);

#endif	/* ENERGYINFO_H */

