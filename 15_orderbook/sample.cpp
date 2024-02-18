#include <iostream>
#include <map>

using namespace std;

#define NUM_ENTRIES 1000000

class OrderBook {
  multimap<int, int> orders;

 public:
  void add(int price, int size) { orders.emplace(price, size); }
  void remove(int pos) {
    auto it = orders.begin();
    advance(it, pos);
    orders.erase(it);
  }
  int buy(int shares) {
    int cost = 0;
    while (shares > 0) {
      auto it = orders.begin();
      int size = min(shares, it->second);
      shares -= size;
      cost += size * it->first;
      it->second -= size;
      if (it->second == 0)
        orders.erase(it);
    }
    return cost;
  }
  size_t size() const { return orders.size(); }
};

int main() {
  OrderBook orderbook;

  for (int i = 0; i < NUM_ENTRIES; i++) {
    char c;
    cin >> c;
    switch (c) {
      case '+':
        int size, price;
        cin >> price >> size;
        orderbook.add(price, size);
        break;

      case '-':
        int pos;
        cin >> pos;
        orderbook.remove(pos);
        break;

      case '=':
        cin >> size;
        orderbook.buy(size);
        break;
    }
  }

  // buy 1000 shares and report the total cost
  cout << orderbook.buy(1000) << endl;
  return 0;
}
