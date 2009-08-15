// Copyright (C) 2006-2008 Rosen Diankov (rdiankov@cs.cmu.edu)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#ifndef  BASICRRT_PLANNER_H
#define  BASICRRT_PLANNER_H

#include <sstream>

class RrtPlanner : public PlannerBase
{
public:
    struct Node
    {
        Node() { parent = -1; }
        Node(const Node& r) {
            assert(0); // can't do it since don't know size
        }
             
        int parent;
        dReal q[0]; // the configuration immediately follows the struct
    };

    class SpatialTree
    {
    public:
        enum ExtendType {
            ET_Failed=0,
            ET_Sucess=1,
            ET_Connected=2
        };

        SpatialTree();
        ~SpatialTree(){};

        void Reset(RrtPlanner* planner, int dof=0);

        int AddNode(int parent, const dReal* pfConfig);
        int GetNN(const dReal* q); ///< return the nearest neighbor

        /// extends toward pNewConfig
        /// \param bOneStep if true, extend for only one step
        /// \return true if extension reached pNewConfig
        ExtendType Extend(const dReal* pNewConfig, bool bOneStep, int& lastindex);

        inline int GetDOF() { return _dof; }

        vector<Node*> _nodes;

        DistanceMetric* _pDistMetric;

        dReal _fBestDist; ///< valid after a call to GetNN
        dReal _fStepLength;

    private:
        vector<dReal> _vNewConfig;
        RrtPlanner* _planner;
        int _dof;
    };

    class SimpleDistMetric : public PlannerBase::DistanceMetric
    {
    public:
        SimpleDistMetric() : PlannerBase::DistanceMetric() { thresh = 0.01f; _robot = NULL; }

        virtual void SetRobot(RobotBase* robot)
        {
            _robot = robot;
            if( _robot == NULL )
                return;

            float ftransweight = 2;
            weights.resize(0);
            vector<int>::const_iterator it;
            FORIT(it, _robot->GetActiveJointIndices()) weights.push_back(_robot->GetJointWeight(*it));
            if( _robot->GetAffineDOF() & RobotBase::DOF_X ) weights.push_back(ftransweight);
            if( _robot->GetAffineDOF() & RobotBase::DOF_Y ) weights.push_back(ftransweight);
            if( _robot->GetAffineDOF() & RobotBase::DOF_Z ) weights.push_back(ftransweight);
            if( _robot->GetAffineDOF() & RobotBase::DOF_RotationAxis ) weights.push_back(ftransweight);
            else if( _robot->GetAffineDOF() & RobotBase::DOF_RotationQuat ) {
                weights.push_back(0.4f);
                weights.push_back(0.4f);
                weights.push_back(0.4f);
                weights.push_back(0.4f);
            }
        }

        virtual float Eval(const void* c0, const void* c1)
        {
            assert( _robot->GetActiveDOF() == (int)weights.size() );

            float out = 0;
            for(int i=0; i < _robot->GetActiveDOF(); i++)
                out += weights[i] * (((dReal *)c0)[i]-((dReal *)c1)[i])*(((dReal *)c0)[i]-((dReal *)c1)[i]);
            
            return sqrtf(out);
        }

        vector<dReal> weights;
    };

    class RrtSampleFunction : public PlannerBase::SampleFunction
    {
    public:
        RrtSampleFunction() {}

        void Init(RobotBase* robot) {
            assert( robot != NULL );
            robot->GetActiveDOFLimits(lower, upper);
            range.resize(lower.size());
            for(int i = 0; i < (int)range.size(); ++i)
                range[i] = upper[i] - lower[i];
        }
        virtual void Sample(dReal* pNewSample) {
            for (int i = 0; i < (int)lower.size(); i++) {
                pNewSample[i] = lower[i] + RANDOM_FLOAT()*range[i];
            }
        }

        virtual bool Sample(dReal* pNewSample, const dReal* pCurSample, dReal fRadius) {

            if( pCurSample == NULL ) {
                Sample(pNewSample);
                return true;
            }

            fRadius *= 2;
            for (int i = 0; i < (int)lower.size(); i++) {
                pNewSample[i] = pCurSample[i] + (RANDOM_FLOAT()-0.5f)*fRadius;
                if( pNewSample[i] < lower[i] )
                    pNewSample[i] = lower[i];
                else if( pNewSample[i] > upper[i] )
                    pNewSample[i] = upper[i];
            }
            return true;
        }

    private:
        vector<dReal> lower, upper, range;
    };

    /// used to set the configuration state and gets its degrees of freedom
    class ActiveConfigurationState : public PlannerBase::ConfigurationState
    {
    public:
        ActiveConfigurationState() : robot(NULL) {}
        virtual void SetRobot(RobotBase* probot) { robot = probot; }
        virtual void SetState(const dReal* pstate) { robot->SetActiveDOFValues(NULL, pstate); }
        virtual void GetState(dReal* pstate) { robot->GetActiveDOFValues(pstate); }
        virtual void GetLimits(dReal* plower, dReal* pupper) { robot->GetActiveDOFLimits(plower, pupper); }
        
        virtual int GetDOF() const { return robot->GetActiveDOF(); }

    private:
        RobotBase* robot;
    };

    RrtPlanner(EnvironmentBase* penv);
    ~RrtPlanner();

    virtual bool InitPlan(RobotBase* pbase,const PlannerParameters* pparams);//, ColChecker* pCC);
    virtual bool PlanPath(Trajectory* ptraj, std::ostream* pOutStream);

    virtual RobotBase* GetRobot() const { return _pRobot; }
    virtual const wchar_t* GetDescription() const { return L"Basic Goal-Biased RRT Planner"; }

    // interval types   ( , )      ( , ]       [ , )      [ , ]
    enum IntervalType { OPEN=0,  OPEN_START,  OPEN_END,  CLOSED };

private:

    bool _CheckCollision(const dReal* pQ0, const dReal* pQ1, IntervalType interval, vector< vector<dReal> >* pvCheckedConfigurations = NULL);

    /// check collision between body and environment
    bool _CheckCollision(const dReal *pConfig, bool bReport = false);

    /// optimize the computed path over a number of iterations
    void _OptimizePath(list<Node*>& path);

    int GetDOF() const { return _parameters.pConfigState->GetDOF(); }

    PlannerParameters _parameters;

    RobotBase*         _pRobot;
    float _fGoalBiasProb;
    bool _bOneStep;

    std::vector< std::vector<dReal> > _vecGoals;

    std::vector<dReal>         _randomConfig;  //!< chain configuration

    std::vector<dReal>          _jointResolution;  //!< joint discretization
    std::vector<dReal>          _jointResolutionInv;
    std::vector<dReal>          _jointIncrement;

    std::vector<dReal>          _lowerLimit;  //!< joint limits
    std::vector<dReal>          _upperLimit;
    std::vector<dReal>          _validRange;

    SpatialTree _tree;
    
    SimpleDistMetric _defaultdist;
    RrtSampleFunction _defaultsamplefn;
    ActiveConfigurationState _defaultstate;

    bool bInit;
};

#ifdef RAVE_REGISTER_BOOST
#include BOOST_TYPEOF_INCREMENT_REGISTRATION_GROUP()

BOOST_TYPEOF_REGISTER_TYPE(RrtPlanner::Node)
BOOST_TYPEOF_REGISTER_TYPE(RrtPlanner::SpatialTree)

#endif

#endif   // BASICRRT_PLANNER_H

