#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>

using eosio::asset;
using eosio::indexed_by;
using eosio::const_mem_fun;
using eosio::permission_level;
using eosio::action;
using eosio::print;

class trump : public eosio::contract{
    private:
        //@abi table project i64
        struct project{
            uint64_t id;
            asset minBet;
            uint64_t totalBet;
            uint32_t numberOfBets;
            uint64_t startTime;
            bool isEnd;

            uint64_t primary_key() const { return id; }
            EOSLIB_SERIALIZE(project, (id)(minBet)(totalBet)(numberOfBets)(startTime)(isEnd))
        };

        typedef eosio::multi_index<N(project), project> project_table;

        project_table _projects;
    public:
        trump(account_name self):eosio::contract(self),
            _projects(_self, _self)
            {};

        //@abi action
        void create(const asset& _minBet, uint64_t _startTime){
            require_auth(_self);
            auto cur_project = _projects.begin();

            if(cur_project != _projects.end()){
                eosio_assert(cur_project->isEnd == true, "not brgin");
            }

            _projects.emplace(_self, [&](auto & row){
                row.id = _projects.available_primary_key();
                row.minBet = _minBet;
                row.startTime = _startTime;
                row.totalBet = 0;
                row.numberOfBets = 0;
                row.isEnd = false;
            });
        }

        //@abi action
        void offerbet(const asset& bet,const account_name user, uint64_t timeSelect, uint64_t timeSend, uint64_t pid){
            require_auth(_self);
            auto _end = _projects.find(pid);

            eosio_assert(_end != _projects.end(), "not begin");

            _projects.modify(_end, _self, [&](auto & row){
                row.numberOfBets += 1;
                row.totalBet += bet.amount;
            });
            
        }

        //@abi action
        void endproject(){
            require_auth(_self);
            auto _end = _projects.begin();

            while( _end != _projects.end() && _end->isEnd == 1){
                _end++;
            }
            _projects.modify(_end, _self, [&](auto & row){
                row.isEnd = true;
            });
        }

        //@abi action
        void transferbet(const account_name to, const asset& quantity){
            require_auth( _self );
 
            eosio_assert( quantity.is_valid(), "invalid quantity" );
            eosio_assert( quantity.amount > 0, "must withdraw positive quantity" );

            action(
                permission_level{ _self, N(active) },
                N(eosio.token), N(transfer),
                std::make_tuple(_self, to, quantity, std::string(""))
            ).send();
        }

        //@abi action
        void reset() {
            require_auth( _self );

            auto ite = _projects.begin();
            while( ite != _projects.end()) {
                ite = _projects.erase(ite);
            }
        }
};
EOSIO_ABI(trump, (create)(offerbet)(endproject)(transferbet)(reset))