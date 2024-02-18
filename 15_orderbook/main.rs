#[link(name = "c")]
extern {
    fn mmap(addr: *mut u8, len: usize, prot: i32, flags: i32, fd: i32, offset: i64) -> *mut u8;
    // fn madvise(addr: *mut u8, len: usize, advice: i32) -> i32;
    fn __errno_location() -> *const i32;
    fn lseek(fd: i32, offset: i64, whence: i32) -> i64;
    fn open(path: *const u8, oflag: i32) -> i32;
}

#[allow(dead_code)]
unsafe fn mmap_stdin<'a>() -> &'a [u8] {
    mmap_fd(0)
}

#[allow(dead_code)]
unsafe fn mmap_path<'a>(path: &str) -> &'a [u8] {
    let mut path2 = vec![];
    path2.extend_from_slice(path.as_bytes());
    path2.push(0);
    let fd = open(path2.as_ptr(), 0);
    if fd == -1 {
        panic!("open failed, errno {}", *__errno_location());
    }
    mmap_fd(fd)
}

unsafe fn mmap_fd<'a>(fd: i32) -> &'a [u8] {
    let seek_end = 2;
    let size = lseek(fd, 0, seek_end);
    if size == -1 {
        panic!("lseek failed, errno {}", *__errno_location());
    }
    let prot_read = 0x01;
    let map_private = 0x02;
    let map_populate = 0x08000;
    // let madv_hugepage = 0x0E; 
    // let ptr = madvise(0 as _, size as usize, madv_hugepage);
    // if ptr as isize == -1 {
    //     panic!("madvise failed, errno {}", *__errno_location());
    // }
    let ptr = mmap(0 as _, size as usize, prot_read, map_private | map_populate, fd, 0);
    if ptr as isize == -1 {
        panic!("mmap failed, errno {}", *__errno_location());
    }
    std::slice::from_raw_parts(ptr, size as usize)
}

#[derive(Debug, PartialOrd, Ord, PartialEq, Eq)]
struct Order {
    price: u32,
    size: u32,
}

struct Orderbook {
    // orders: Vec<Order>
    orders: BTreeSet<Order>
}

impl Orderbook {
    fn new() -> Self {
        Self { orders: Vec::with_capacity(u32::MAX as usize) }
    }

    fn add(&mut self, price: u32, size: u32) {
        let order = Order { price, size };
        // println!("add: {} {}", price, size);

        // if first order
        if self.orders.len() == 0 {
            self.orders.push(order);
            return;
        }

        // check with the last order
        if self.orders.len() > 1 {
            let last = &self.orders[self.orders.len() - 1];

            // if bigger than the last we can just insert at the back
            if price > last.price || (price == last.price && size >= last.size) {
                self.orders.push(order);
                return;
            }

            // if same price as last but smaller size, we find the pos from back
            if price == last.price {
                for (i, o) in self.orders.iter().rev().enumerate() {
                    if price == o.price && size < o.size {
                        continue 
                    } else {
                        self.orders.insert(i + 1, order);
                        return;
                    }
                }
            }
        }

        let best_ask = &self.orders[0];

        // smaller price, we insert in front
        if price < best_ask.price || (price == best_ask.price && size < best_ask.size) {
            self.orders.insert(0, order);
            return;
        }

        // if same price but larger size then we find the next position for the size
        if price == best_ask.price {
            for i in 1..self.orders.len() {
                if price == self.orders[i].price && size > self.orders[i].size {
                    continue;
                } else {
                    self.orders.insert(i, order);
                    return;
                }
            }
        }

        // bigger price, so we advance and find the position
        for i in 1..self.orders.len() {
            if price > self.orders[i].price {
                continue;
            } else if price == self.orders[i].price && size > self.orders[i].size {
                continue;
            } else {
                self.orders.insert(i, order);
                return;
            }
        }

        // if not, worst case we add at the back
        self.orders.push(order);
    }

    fn remove(&mut self, pos: usize) {
        // println!("remove: {}", pos);
        self.orders.remove(std::cmp::min(pos, self.orders.len() - 1));
    }

    fn buy(&mut self, mut shares: u32) -> u32 {
        // println!("buy: {}", shares);
        let mut cost = 0;
        while shares > 0 && self.orders.len() > 0 {
            let order = &mut self.orders[0];
            let size = std::cmp::min(shares, order.size);
            shares -= size;
            cost += size * order.price;
            order.size -= size;
            if order.size == 0 {
                self.orders.remove(0);
            }
        }
        cost
    }

    #[allow(unused)]
    fn print(&self) {
        println!("{:?}", self.orders);
    }
}

enum Action {
    Add { price: u32, size: u32 },
    Remove { pos: usize },
    Buy { shares: u32 }
}

impl Action {
    fn from_str(string: &str) -> Self {
        let mut splits = string.trim().split(' ');
        match splits.next().unwrap() {
            "+" => {
                let price = splits.next().unwrap().parse().unwrap();
                let size = splits.next().unwrap().parse().unwrap();
                Self::Add { price, size }
            },
            "-" => {
                let pos = splits.next().unwrap().parse().unwrap();
                Self::Remove { pos }
            },
            "=" => {
                let shares = splits.next().unwrap().parse().unwrap();
                Self::Buy { shares }
            },
            _ => panic!("unsupported operation"),
        }
    }
}

fn main() {
    let mut orderbook = Orderbook::new();
    let buffer = std::str::from_utf8(unsafe { mmap_stdin() }).unwrap();
    let parts = buffer.split("\n");

    for part in parts {
        if part == "" {
            continue
        }
        match Action::from_str(&part) {
            Action::Add { price, size } => orderbook.add(price, size),
            Action::Remove { pos } => orderbook.remove(pos),
            Action::Buy { shares } => { orderbook.buy(shares); },
        };
        // orderbook.print();
    }

    println!("{}", orderbook.buy(1000));
}