#include <iostream>
#include <vector>

using namespace std;

#define NUM_ENTRIES 1000000
#define MAX_DEPTH 1 << 24

#if __linux__
#include <linux/version.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...) ;
#endif

struct Order {
  int price, size;
};

class OrderBook {
  std::vector<Order> orders;

 public:
  OrderBook() {
    orders.reserve(MAX_DEPTH);
    DEBUG_PRINT("reserved orders with %d\n", MAX_DEPTH);
  }

  void add(int price, int size) {
    Order order = Order{.price = price, .size = size};

    // if first order
    if (orders.size() == 0) {
      orders.push_back(order);
      return;
    }

    // check with the last order
    if (orders.size() > 1) {
      DEBUG_PRINT("checking worst ask\n");
      auto last = *(orders.end() - 1);
      // if bigger than the last we can just insert at the back
      if (price > last.price || (price == last.price && size >= last.size)) {
        DEBUG_PRINT("inserting at back\n");
        orders.push_back(order);
        return;
      }
      // if same price as last but smaller size, we find the pos from back
      else if (price == last.price) {
        DEBUG_PRINT("same price as last, reversing to find pos\n");
        for (int i = orders.size() - 2; i >= 0; i--)
          if (price == orders[i].price && size < orders[i].size)
            continue;
          else {
            DEBUG_PRINT("inserting at: %d\n", i + 1);
            orders.insert(orders.begin() + i + 1, order);
            return;
          }
      }
    }

    DEBUG_PRINT("checking best_ask\n");
    auto best_ask = orders[0];

    // smaller price, we insert in front
    if (price < best_ask.price ||
        // if equal price and size is correct, we also insert front
        (price == best_ask.price && size < best_ask.size)) {
      orders.insert(orders.begin(), order);
      return;
    }
    // if same price but larger size then we find the next position for the size
    else if (price == best_ask.price) {
      for (int i = 1; i < orders.size(); i++) {
        if (price == orders[i].price && size > orders[i].size)
          continue;
        else {
          orders.insert(orders.begin() + i, order);
          return;
        }
      }
    }

    // bigger price, so we advance and find the position
    DEBUG_PRINT("bigger price\n");
    for (int i = 1; i < orders.size(); i++)
      if (price > orders[i].price)
        continue;
      else if (price == orders[i].price && size > orders[i].size)
        continue;
      else {
        DEBUG_PRINT("inserting at %d\n", i);
        orders.insert(orders.begin() + i, order);
        return;
      }

    // if not, worst case we add at the back
    orders.push_back(order);
  }

  void remove(int pos) {
    orders.erase(orders.begin() + min(orders.size() - 1, (size_t)pos));
  }

  int buy(int shares) {
    int cost = 0;
    while (shares > 0 && orders.size() > 0) {
      auto it = orders.begin();
      int size = min(shares, it->size);
      shares -= size;
      cost += size * it->price;
      DEBUG_PRINT("it: (%d, %d), size: %d, cost: %d\n", it->price, it->size,
                  size, cost);
      it->size -= size;
      if (it->size == 0)
        orders.erase(it);
    }
    return cost;
  }

  void print() {
    for (auto it = orders.begin(); it != orders.end(); it++) {
      DEBUG_PRINT("(%d, %d)", it->price, it->size);
      if (it != orders.end() - 1)
        DEBUG_PRINT(", ");
    }
    if (orders.size() > 0)
      DEBUG_PRINT("\n");
  }
};

int i = 0;
char c;
OrderBook orderbook;

#if __linux__
#define OP_ADD_GET_PRICE 0
#define OP_ADD_GET_SIZE 1
#define OP_REMOVE_POS 2
#define OP_BUY_SHARES 3

int main() {
  off_t fsize = lseek(0, 0, SEEK_END);
  char* buffer =
      (char*)mmap(0, fsize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, 0, 0);

  uint8_t state = OP_ADD_GET_PRICE;
  std::string token;
  int price, size, pos, shares;

  for (int i = 0; i < fsize; i++) {
    switch (buffer[i]) {
      case '+':
        state = OP_ADD_GET_PRICE;
        i++;
        continue;
      case '-':
        state = OP_REMOVE_POS;
        i++;
        continue;
      case '=':
        state = OP_BUY_SHARES;
        i++;
        continue;
      default:
        break;
    }

    switch (state) {
      case OP_ADD_GET_PRICE:
        if (buffer[i] == ' ') {
          price = stoi(token);
          token = "";
          state = OP_ADD_GET_SIZE;
          continue;
        }
        token += buffer[i];
        break;
      case OP_ADD_GET_SIZE:
        if (buffer[i] == '\n') {
          size = stoi(token);
          token = "";
          orderbook.add(price, size);
          continue;
        }
        token += buffer[i];
        break;
      case OP_REMOVE_POS:
        if (buffer[i] == '\n') {
          pos = stoi(token);
          token = "";
          orderbook.remove(pos);
          continue;
        }
        token += buffer[i];
        break;
      case OP_BUY_SHARES:
        if (buffer[i] == '\n') {
          shares = stoi(token);
          token = "";
          orderbook.buy(shares);
          continue;
        }
        token += buffer[i];
        break;
    }
  }

  // buy 1000 shares and report the total cost
  cout << orderbook.buy(1000) << endl;

  return 0;
}
#endif

#if __APPLE__
int main() {
  ios_base::sync_with_stdio(false);
  cin.tie(NULL);

  for (; i < NUM_ENTRIES; i++) {
    cin >> c;
    // DEBUG_PRINT("i: %d, got: %c", i, c);
    switch (c) {
      case '+':
        int size, price;
        cin >> price >> size;
        // DEBUG_PRINT(" %d %d\n", price, size);
        orderbook.add(price, size);
        break;

      case '-':
        int pos;
        cin >> pos;
        // DEBUG_PRINT(" %d\n", pos);
        orderbook.remove(pos);
        break;

      case '=':
        cin >> size;
        // DEBUG_PRINT(" %d\n", size);
        orderbook.buy(size);
        break;
    }
    // orderbook.print();
  }

  // buy 1000 shares and report the total cost
  cout << orderbook.buy(1000) << endl;
  return 0;
}
#endif