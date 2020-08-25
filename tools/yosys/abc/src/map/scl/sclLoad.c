/**CFile****************************************************************

  FileName    [sclLoad.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Standard-cell library representation.]

  Synopsis    [Wire/gate load computations.]

  Author      [Alan Mishchenko, Niklas Een]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - August 24, 2012.]

  Revision    [$Id: sclLoad.c,v 1.0 2012/08/24 00:00:00 alanmi Exp $]

  Modified By [Soheil Hashemi, Marina Neseem]

  Affiliation [Brown University]

  Date        [Started February 2019.]


***********************************************************************/

#include "sclSize.h"
#include "base/main/main.h"
#include "base/cmd/cmd.h"
#include <sys/types.h>
#include <unistd.h>


ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Returns estimated wire capacitances for each fanout count.]

  Description []
               
  SideEffects []`

  SeeAlso     []

***********************************************************************/
Vec_Flt_t * Abc_SclFindWireCaps( SC_WireLoad * pWL, int nFanoutMax )
{
    Vec_Flt_t * vCaps = NULL;
    float EntryPrev, EntryCur, Slope;
    int i, iPrev, k, Entry, EntryMax;
    assert( pWL != NULL );
    // find the biggest fanout count
    EntryMax = 0;
    Vec_IntForEachEntry( &pWL->vFanout, Entry, i )
        EntryMax = Abc_MaxInt( EntryMax, Entry );
    // create the array
    vCaps = Vec_FltStart( Abc_MaxInt(nFanoutMax, EntryMax) + 1 );
    Vec_IntForEachEntry( &pWL->vFanout, Entry, i )
        Vec_FltWriteEntry( vCaps, Entry, Vec_FltEntry(&pWL->vLen, i) * pWL->cap );
    if ( Vec_FltEntry(vCaps, 1) == 0 )
        return vCaps;
    // interpolate between the values
    assert( Vec_FltEntry(vCaps, 1) != 0 );
    iPrev = 1;
    EntryPrev = Vec_FltEntry(vCaps, 1);
    Vec_FltForEachEntryStart( vCaps, EntryCur, i, 2 )
    {
        if ( EntryCur == 0 )
            continue;
        Slope = (EntryCur - EntryPrev) / (i - iPrev);
        for ( k = iPrev + 1; k < i; k++ )
            Vec_FltWriteEntry( vCaps, k, EntryPrev + Slope * (k - iPrev) );
        EntryPrev = EntryCur;
        iPrev = i;
    }
    // extrapolate after the largest value
    Slope = pWL->cap * pWL->slope;
    for ( k = iPrev + 1; k < i; k++ )
        Vec_FltWriteEntry( vCaps, k, EntryPrev + Slope * (k - iPrev) );
    // show
//    Vec_FltForEachEntry( vCaps, EntryCur, i )
//        printf( "%3d : %f\n", i, EntryCur );
    return vCaps;
}

/**Function*************************************************************

  Synopsis    [Computes load for all nodes in the network.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
float Abc_SclFindWireLoad( Vec_Flt_t * vWireCaps, int nFans )
{
    if ( vWireCaps == NULL )
        return 0;
    return Vec_FltEntry( vWireCaps, Abc_MinInt(nFans, Vec_FltSize(vWireCaps)-1) );
}
float Abc_SclFindSpefWireLoad( st__table * tNameToCap, Abc_Obj_t * pObj )
{
    Abc_Obj_t * pFanout;
    float Load = 0;
    int i;
    Abc_ObjForEachFanout(pObj,pFanout,i)
    {
        char * pWireCapString = 0;
        char * pWireName = Abc_ObjName(pFanout);
        st__lookup(tNameToCap, pWireName, &pWireCapString);
        if(pWireCapString != NULL)
        {
            Load += (float)atof(pWireCapString); // Should make unit equator (in our example, SPEF is in PF and Lib is in FF)
        }
    }
    return Load;
}
float Abc_SclFindSpefWireLoadStop( st__table * tNameToCap,Vec_Ptr_t * vFanouts, int iStop )
{
    Abc_Obj_t * pFanout;
    float Load = 0;
    int i;

    Vec_PtrForEachEntryStop(Abc_Obj_t *, vFanouts, pFanout, i, iStop )
    {
        char * pWireCapString = 0;
        char * pWireName = Abc_ObjName(pFanout);
        st__lookup(tNameToCap, pWireName, &pWireCapString);
        if(pWireCapString != NULL)
        {
            Load += (float)atof(pWireCapString);
        }
    }
    return Load;
}
void Abc_SclAddWireLoad( SC_Man * p, Abc_Obj_t * pObj, int fSubtr )
{
    float Load = Abc_SclFindWireLoad( p->vWireCaps, Abc_ObjFanoutNum(pObj) );
    Abc_SclObjLoad(p, pObj)->rise += fSubtr ? -Load : Load;
    Abc_SclObjLoad(p, pObj)->fall += fSubtr ? -Load : Load;
    //printf("Printing WLM names: %s, and adding load: %f \n", Abc_ObjName(pObj), Load);
}
void Abc_SclAddSpefWireLoad( SC_Man * p, Abc_Obj_t * pObj,float Load, int fSubtr )
{
    Abc_SclObjLoad(p, pObj)->rise += fSubtr ? -Load : Load;
    Abc_SclObjLoad(p, pObj)->fall += fSubtr ? -Load : Load;
}
void Abc_SclComputeLoad( SC_Man * p )
{
    Abc_Obj_t * pObj, * pFanin;
    int i, k;
    // clear load storage
    Abc_NtkForEachObj( p->pNtk, pObj, i )
    {
        SC_Pair * pLoad = Abc_SclObjLoad( p, pObj );
        if ( !Abc_ObjIsPo(pObj) )
            pLoad->rise = pLoad->fall = 0.0;
    }
    // add cell load
    Abc_NtkForEachNode1( p->pNtk, pObj, i )
    {
        SC_Cell * pCell = Abc_SclObjCell( pObj );
        Abc_ObjForEachFanin( pObj, pFanin, k )
        {
            SC_Pair * pLoad = Abc_SclObjLoad( p, pFanin );
            SC_Pin * pPin = SC_CellPin( pCell, k );
            pLoad->rise += pPin->rise_cap;
            pLoad->fall += pPin->fall_cap;
        }
    }
    // add PO load
    Abc_NtkForEachCo( p->pNtk, pObj, i )
    {
        SC_Pair * pLoadPo = Abc_SclObjLoad( p, pObj );
        SC_Pair * pLoad = Abc_SclObjLoad( p, Abc_ObjFanin0(pObj) );
        pLoad->rise += pLoadPo->rise;
        pLoad->fall += pLoadPo->fall;
    }
    // add wire load
    if ( p->pWLoadUsed != NULL )
    {
        if ( p->vWireCaps == NULL )
            p->vWireCaps = Abc_SclFindWireCaps( p->pWLoadUsed, Abc_NtkGetFanoutMax(p->pNtk) );
        Abc_NtkForEachNode1( p->pNtk, pObj, i )
            Abc_SclAddWireLoad( p, pObj, 0 );
        Abc_NtkForEachPi( p->pNtk, pObj, i )
            Abc_SclAddWireLoad( p, pObj, 0 );
    }
    // add wire load using SPEF File
    if ( p->fUseSpef != 0 )
    {
        st__table * tNameToCap = Abc_SclGetSpefNameCapTable(p->pNtk,p->pLib->unit_cap_snd);
        char * pWireName = 0;
        char * pWireCapString = 0;

        printf("-- Adding Capacitances.\n");

        Abc_NtkForEachNode1( p->pNtk, pObj, i )
        {
            float dLoad = 0;
            pWireCapString = 0;
            //pWireName = Abc_ObjName(Abc_ObjFanout0(pObj));
            pWireName = Abc_ObjName(pObj);
            st__lookup(tNameToCap, pWireName, &pWireCapString);
            if(pWireCapString != NULL)
                dLoad = (float)atof(pWireCapString);
            //printf("Node wire name: %s, adding wire load: %f \n", pWireName, dLoad);
            Abc_SclAddSpefWireLoad( p, pObj, dLoad, 0 );
        }
        Abc_NtkForEachPi( p->pNtk, pObj, i )
        {
            float dLoad = 0;
            pWireCapString = 0;
            //pWireName = Abc_ObjName(Abc_ObjFanout0(pObj));
            pWireName = Abc_ObjName(pObj);
            st__lookup(tNameToCap, pWireName, &pWireCapString);
            if(pWireCapString != NULL)
                dLoad = (float)atof(pWireCapString);
            //printf("Pi wire name: %s, adding wire load: %f \n", pWireName, dLoad);
            Abc_SclAddSpefWireLoad( p, pObj, dLoad, 0 );
        }
    }
    // check input loads
    if ( p->vInDrive != NULL )
    {
        Abc_NtkForEachPi( p->pNtk, pObj, i )
        {
            SC_Pair * pLoad = Abc_SclObjLoad( p, pObj );
            if ( Abc_SclObjInDrive(p, pObj) != 0 && (pLoad->rise > Abc_SclObjInDrive(p, pObj) || pLoad->fall > Abc_SclObjInDrive(p, pObj)) )
                printf( "Maximum input drive strength is exceeded at primary input %d.\n", i );
        }
    }
/*
    // transfer load from barbufs
    Abc_NtkForEachBarBuf( p->pNtk, pObj, i )
    {
        SC_Pair * pLoad = Abc_SclObjLoad( p, pObj );
        SC_Pair * pLoadF = Abc_SclObjLoad( p, Abc_ObjFanin(pObj, 0) );
        SC_PairAdd( pLoadF, pLoad );
    }
*/
    // calculate average load
//    if ( p->EstLoadMax )
    {
        double TotalLoad = 0;
        int nObjs = 0;
        Abc_NtkForEachNode1( p->pNtk, pObj, i )
        {
            SC_Pair * pLoad = Abc_SclObjLoad( p, pObj );
            TotalLoad += 0.5 * pLoad->fall + 0.5 * pLoad->rise;
            nObjs++;
        }
        Abc_NtkForEachPi( p->pNtk, pObj, i )
        {
            SC_Pair * pLoad = Abc_SclObjLoad( p, pObj );
            TotalLoad += 0.5 * pLoad->fall + 0.5 * pLoad->rise;
            nObjs++;
        }
        p->EstLoadAve = (float)(TotalLoad / nObjs);
//        printf( "Average load = %.2f\n", p->EstLoadAve );
    }
}

/**Function*************************************************************

  Synopsis    [Updates load of the node's fanins.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Abc_SclUpdateLoad( SC_Man * p, Abc_Obj_t * pObj, SC_Cell * pOld, SC_Cell * pNew )
{
    Abc_Obj_t * pFanin;
    int k;
    Abc_ObjForEachFanin( pObj, pFanin, k )
    {
        SC_Pair * pLoad = Abc_SclObjLoad( p, pFanin );
        SC_Pin * pPinOld = SC_CellPin( pOld, k );
        SC_Pin * pPinNew = SC_CellPin( pNew, k );
        pLoad->rise += pPinNew->rise_cap - pPinOld->rise_cap;
        pLoad->fall += pPinNew->fall_cap - pPinOld->fall_cap;
    }
}
void Abc_SclUpdateLoadSplit( SC_Man * p, Abc_Obj_t * pBuffer, Abc_Obj_t * pFanout )
{
    SC_Pin * pPin;
    SC_Pair * pLoad;
    int iFanin = Abc_NodeFindFanin( pFanout, pBuffer );
    assert( iFanin >= 0 );
    assert( Abc_ObjFaninNum(pBuffer) == 1 );
    pPin = SC_CellPin( Abc_SclObjCell(pFanout), iFanin );
    // update load of the buffer
    pLoad = Abc_SclObjLoad( p, pBuffer );
    pLoad->rise -= pPin->rise_cap;
    pLoad->fall -= pPin->fall_cap;
    // update load of the fanin
    pLoad = Abc_SclObjLoad( p, Abc_ObjFanin0(pBuffer) );
    pLoad->rise += pPin->rise_cap;
    pLoad->fall += pPin->fall_cap;
}

/**Function*************************************************************

  Synopsis    [Convert cap Units from SPEF file unit to Library unit.]

  Description [The funtion takes unit from the spef file as a char* and
               the unit from library as int num, for example if unit in SPEF
               was in pico farad and Library is in Fento Farad, then to do the
               conversion the function should take unit = "PF" or "pf" and should
               take the target unit as 15.]

  SideEffects []

  SeeAlso     [This function currently does not support having unit multipliers
               or any units other than nano, pico and femto]

***********************************************************************/
char * Abc_UnitConvert(char* num, char* unit, int targetUnit,char* errorMsg)
{
    float outputResult = 0;
    switch (targetUnit) {
    case 9:
        if(strcasecmp(unit,"nf") == 0)
            outputResult = atof(num)*pow(10,1);
        else if(strcasecmp(unit,"pf") == 0)
            outputResult = atof(num)/pow(10,3);
        else if(strcasecmp(unit,"ff") == 0)
            outputResult = atof(num)/pow(10,6);
        else
            errorMsg = "Error: Convert SPEF Cap_Unit failed, this unit is not supported";
        break;
    case 12:
        if(strcasecmp(unit,"nf") == 0)
            outputResult = atof(num)*pow(10,3);
        else if(strcasecmp(unit,"pf") == 0)
            outputResult = atof(num)*pow(10,1);
        else if(strcasecmp(unit,"ff") == 0)
            outputResult = atof(num)/pow(10,3);
        else
            errorMsg = "Error: Convert SPEF Cap_Unit failed, this unit is not supported";
        break;
    case 15:
        if(strcasecmp(unit,"nf") == 0)
            outputResult = atof(num)*pow(10,6);
        else if(strcasecmp(unit,"pf") == 0)
            outputResult = atof(num)*pow(10,3);
        else if(strcasecmp(unit,"ff") == 0)
            outputResult = atof(num)*pow(10,1);
        else
            errorMsg = "Error: Convert SPEF Cap_Unit failed, this unit is not supported";
        break;
    default:
            errorMsg = "Error: Convert SPEF Cap_Unit failed, this unit in the library is not supported";
        break;
    }
    char CoutputResult[10];
    sprintf(CoutputResult,"%f",outputResult);
    return Abc_UtilStrsav(CoutputResult);

}

/**Function*************************************************************

  Synopsis    [Parse Wire capacitances from SPEF.]

  Description [The funtion reads a SPEF file as the input and returens
               a st__table as the output table hasing wire names to 
               capacitances.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Abc_SclParseSpef( char * pFilename, st__table * tNameToCap, int targetCapUnit)
{
    char Buffer[1000], Buffer2[1000];
    char * pToken;
    int nLines = 0;
    int fHasNameMap = 0;
    char * name = NULL;
    char * cap = NULL;
    char * capUnit = NULL;
    char * capUnitMultiplier = NULL;
    char * num = NULL;
    FILE *pFile = fopen( pFilename, "r");
    if (pFile == NULL){
        printf("Abc_SclParseSpef() error! The %s file in not avaliable. STOP!\n", pFilename);
        return -1;
    }

    st__table * tNumToName;
    tNumToName = st__init_table(strcmp, st__strhash);

    // Parse the Spef to tNameToNum and tNumToCap hash tables
    while(fgets(Buffer, 1000, pFile))
    {
        nLines++;
        pToken = strtok( Buffer, " \t\r\n");
        if(pToken == NULL)
            continue;
        else {
            if(strcmp(pToken, "*C_UNIT") == 0){
                capUnitMultiplier = Abc_UtilStrsav(strtok(NULL, " \t "));
                capUnit = Abc_UtilStrsav(strtok(NULL, " \n"));
                continue;
            }
            else if(strcmp(pToken, "*NAME_MAP") == 0){
                fHasNameMap = 1;
                int fFoundPorts = 0;
                while (!fFoundPorts){
                    if(!fgets(Buffer2, 1000, pFile)){
                        printf("Unexpected EOF after NAME_MAP token.\n");
                        return -1;
                    }
                    nLines++;
                    pToken = strtok( Buffer2, " \t\r\n");
                    if(pToken == NULL)
                        continue;
                    else{
                        if(strcmp(pToken, "*PORTS") == 0){
                            fFoundPorts = 1;
                            continue;
                        }
                        else{    
                            num = pToken;
                            name = strtok(NULL, " \n");
                            if (st__insert(tNumToName, Abc_UtilStrsav(num), Abc_UtilStrsav(name)) == st__OUT_OF_MEM){
                                printf("writing to tNumToName hash table failed.\n");
                                return -1;
                            }
                        }
                    }
                }

                continue;
            }
            else if(strcmp(pToken, "*D_NET") == 0){
                num = strtok(NULL, " \n");
                cap = strtok(NULL, " \n");

                char * errorMsg = "";
                char * fCap = Abc_UnitConvert(cap,capUnit, targetCapUnit,errorMsg);
                if(strcmp(errorMsg,"") != 0)
                {
                    printf("%s.\n",errorMsg);
                    return -1;
                }

                if(fHasNameMap ==1){
                    st__lookup(tNumToName, num, &name);
                    if (st__insert(tNameToCap, Abc_UtilStrsav(name), Abc_UtilStrsav(fCap)) == st__OUT_OF_MEM){
                        printf("writing to TNameToCap hash table failed.\n");
                        return -1;
                    }
                    continue;
                }
                else{
                    name = num;
                    if (st__insert(tNameToCap, Abc_UtilStrsav(name), Abc_UtilStrsav(fCap)) == st__OUT_OF_MEM){
                        printf("writing to TNameToCap hash table failed.\n");
                        return -1;
                    }
                    continue;
                }
            }
            else{
                continue;
            }
        }
    }
    st__free_table(tNumToName);
    return 1;
}
////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

