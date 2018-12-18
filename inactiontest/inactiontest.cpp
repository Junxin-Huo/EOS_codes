#include <eosiolib/eosio.hpp>

using namespace eosio;

class inactiontest : public contract
{
 public:
   using contract::contract;

   void pay(account_name to, uint64_t value)
   {
      print("| function: pay |");
      address_index ad(_self, _self);
      auto iterator = ad.find(to);
      if (iterator == ad.end())
      {
         print("| emplace |");
         ad.emplace(to, [&](auto &o) {
            o.key = to;
            o.value = value;
         });
      }
      else
      {
         print("| modify |");
         ad.modify(iterator, to, [&](auto &o) {
            o.value = value;
         });
      }
   }

   void erase(account_name user)
   {
      print("| function: erase |");
      address_index ad(_self, _self);
      auto iterator = ad.find(user);
      eosio_assert(iterator != ad.end(), "Record does not exist");
      ad.erase(iterator);
   }

   void hack(account_name to, uint64_t value)
   {
      print("| function: hack |");
      action(
          permission_level{to, N(active)},
          _self,
          N(pay),
          std::make_tuple(to, value))
          .send();
   }

   void normal(uint64_t value)
   {
      print("| function: normal |");
      action(
          permission_level{_self, N(active)},
          _self,
          N(pay),
          std::make_tuple(_self, value))
          .send();
   }

 private:
   // @abi table people i64
   struct person
   {
      account_name key;
      uint64_t value;
      uint64_t primary_key() const { return key; }
   };
   typedef eosio::multi_index<N(people), person> address_index;
};

#undef EOSIO_ABI
#define EOSIO_ABI(TYPE, MEMBERS)                                                                                             \
   extern "C"                                                                                                                \
   {                                                                                                                         \
      void apply(uint64_t receiver, uint64_t code, uint64_t action)                                                          \
      {                                                                                                                      \
         print("| receiver: ", name{receiver});                                                                              \
         print("| code: ", name{code});                                                                                      \
         print("| action: ", name{action});                                                                                  \
         auto self = receiver;                                                                                               \
         if (action == N(onerror))                                                                                           \
         {                                                                                                                   \
            /* onerror is only valid if it is for the "eosio" code account and authorized by "eosio"'s "active permission */ \
            eosio_assert(code == N(eosio), "onerror action's are only valid from the \"eosio\" system account");             \
         }                                                                                                                   \
         if (code == self || action == N(onerror))                                                                           \
         {                                                                                                                   \
            TYPE thiscontract(self);                                                                                         \
            switch (action)                                                                                                  \
            {                                                                                                                \
               EOSIO_API(TYPE, MEMBERS)                                                                                      \
            }                                                                                                                \
            /* does not allow destructor of thiscontract to run: eosio_exit(0); */                                           \
         }                                                                                                                   \
      }                                                                                                                      \
   }

EOSIO_ABI(inactiontest, (pay)(erase)(hack)(normal))
