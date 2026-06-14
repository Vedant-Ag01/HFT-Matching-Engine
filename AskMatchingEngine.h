#include <vector>
#include <unordered_map>
#include <utility>
#include <cstdint>

struct Order {
    int orderId;
    int64_t price;
};

class AskMatchingEngine {
private:
    std::vector<Order> heap;
    std::unordered_map<int, int> position_map; 

    void swapNodes(int indexA, int indexB) {
        position_map[heap[indexA].orderId] = indexB;
        position_map[heap[indexB].orderId] = indexA;
        std::swap(heap[indexA], heap[indexB]);
    }

    void bubbleUp(int current_index) {
        while (current_index > 0 && heap[current_index].price < heap[(current_index - 1) / 2].price) {
            swapNodes(current_index, (current_index - 1) / 2);
            current_index = (current_index - 1) / 2;
        }
    }

    void siftDown(int current_index) {
        int n = heap.size();
        while ((2 * current_index + 1) < n) {
            int left_child = 2 * current_index + 1;
            int right_child = 2 * current_index + 2;
            int smallest_child = left_child;
            
            if (right_child < n && heap[right_child].price < heap[left_child].price) {
                smallest_child = right_child;
            }
            if (heap[current_index].price <= heap[smallest_child].price) break;
            
            swapNodes(current_index, smallest_child);
            current_index = smallest_child;
        }
    }

public:
    // Limit Order: O(log N)
    void insertOrder(int id, int64_t price) {
        if (position_map.count(id)) return;
        heap.push_back({id, price});
        position_map[id] = heap.size() - 1;
        bubbleUp(heap.size() - 1);
    }

    // Market Order: O(log N)
    void executeMin() {
    if (heap.empty()) return;
    position_map.erase(heap[0].orderId);
    if (heap.size() == 1) { heap.pop_back(); return; }
    heap[0] = heap.back();
    position_map[heap[0].orderId] = 0;
    heap.pop_back();
    siftDown(0);
}

    // Cancel Order: O(1) Deletion + O(log N) Repair
    void cancelOrder(int target_id) {
        if (position_map.find(target_id) == position_map.end()) return; 

        int index = position_map[target_id];
        int last_index = heap.size() - 1;

        if (index != last_index) {
            swapNodes(index, last_index);
        }

        position_map.erase(target_id);
        heap.pop_back();
        
        if (index != last_index && index < heap.size()) {
            bubbleUp(index);
            siftDown(index);
        }
    }
    bool isHeapEmpty() const { return heap.empty(); }
};