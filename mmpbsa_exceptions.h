/* 
 * Various exceptions (hopefully not) thrown by mmpbsa classes.
 *
 * Created on June 18, 2010, 8:52 AM
 */
#include <stdexcept>

#ifndef MMPBSA_EXCEPTIONS_H
#define	MMPBSA_EXCEPTIONS_H

//Error types (don't use 0 )
static const int UNKNOWN_ERROR = -1;///<Avoid, as this is vague.
static const int FILE_READ_ERROR = 1;///<IO problem reading prmtop file.
static const int BROKEN_PRMTOP_FILE = 2;///<prmtop file is improperly formatted or missing data.
static const int BROKEN_TRAJECTORY_FILE = 3;///<trajectory file is improperly formatted or missing data.
static const int INVALID_PRMTOP_DATA = 4;///<Data which was loaded into an array is incorrect based on what is expected.
static const int DATA_FORMAT_ERROR = 5;///<Use this error, when data *within* the program no long matches what it should due to formatting problems.

class SanderIOException : public std::runtime_error {
public:
    SanderIOException( const std::string& error) : runtime_error(error) {errorType = UNKNOWN_ERROR;}

    /**
     * Creates an exception, with a specified error type.
     *
     * @param error
     * @param errorType
     */
    SanderIOException(const std::string& error, const int& errorType) : runtime_error(error){this->errorType = errorType;}

    /**
     * Returns the error type, corresponding to the error types listed below.
     * These should be returned if the exception is caught and the program
     * dies gracefully.
     * 
     * @return int error type.
     */
    int getErrType(){return errorType;}

    
private:
    int errorType;

};

class MMPBSAException : public std::runtime_error
{
    public:
        /**
         * Very General MMPBSAException. Use more specific exception if one is
         * available.
         * 
         * @param error
         */
    MMPBSAException( const std::string& error) : runtime_error(error) {errorType = UNKNOWN_ERROR;}

    /**
     * Creates an exception, with a specified error type.
     *
     * @param error
     * @param errorType
     */
    MMPBSAException(const std::string& error, const int& errorType) : runtime_error(error){this->errorType = errorType;}

    /**
     * Returns the error type, corresponding to the error types listed below.
     * These should be returned if the exception is caught and the program
     * dies gracefully.
     *
     * @return int error type.
     */
    int getErrType(){return errorType;}


private:
    int errorType;

};

#endif	/* MMPBSA_EXCEPTIONS_H */

