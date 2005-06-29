class BankAccount: public smoc_actor {
public:
  smoc_port_in<double>  InterestDue, Transaction;
  smoc_port_out<double> Balance;
private:
  double interestRate;
  double state;
  
  void action0() {
    Balance[0] = state;
    state = state * (1 + interestRate) + Transaction[0];
  }
  void action1() {
    Balance[0] = state;
    state = state + Transaction[0];
  }
  void action2() {
    Balance[0] = state;
    state = state * (1 + interestRate);
  }
  smoc_firing_state start;
public:
  BankAccount(sc_module_name name,
              double interestRate = 0.1,
              double initialBalance = 100)
    : smoc_actor(name, start),
      interestRate(interestRate),
      state(initialBalance) {
    start = (InterestDue.getAvailableTokens() >= 1 &&
             Transaction.getAvailableTokens() >= 1   ) >>
            (Balance.getAvailableSpace()      >= 1   ) >>
            call(&BankAccount::action0)                >> start
          | (Transaction.getAvailableTokens() >= 1   ) >>
            (Balance.getAvailableSpace()      >= 1   ) >>
            call(&BankAccount::action1)                >> start
          | (InterestDue.getAvailableTokens() >= 1   ) >>
            (Balance.getAvailableSpace()      >= 1   ) >>
            call(&BankAccount::action2)                >> start;
  }
};
