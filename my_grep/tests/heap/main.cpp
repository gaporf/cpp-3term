#include <bits/stdc++.h>

using namespace std;

const int MAX_VALUE = 2147483647;
const int MIN_VALUE = -2147483648;

struct binomialHeap {
    struct element {
        element *nextSibling, *p, *child;
        int key, index, degree, numHeap;
    } *head;

    binomialHeap() {
        head = nullptr;
    }
};

typedef binomialHeap::element *ptrElement;
typedef binomialHeap *ptrHeap;

int n, curNumber = 0;
vector<ptrHeap> heaps;
vector<ptrElement> elements;
ptrHeap newElement, newHeap;

int getMinimum(ptrHeap heap) {
    if (heap == nullptr) {
        return MIN_VALUE;
    }
    int ans = MAX_VALUE;
    ptrElement element = heap->head;
    while (element != nullptr) {
        ans = min(ans, element->key);
        element = element->nextSibling;
    }
    return ans;
}

ptrHeap merge(ptrHeap a, ptrHeap b, int NUM) {
    if (a == nullptr && b == nullptr) {
        return nullptr;
    }
    if (a == nullptr) {
        ptrElement element = b->head;
        while (element != nullptr) {
            element->numHeap = NUM;
            element = element->nextSibling;
        }
        return b;
    } else if (b == nullptr) {
        ptrElement element = a->head;
        while (element != nullptr) {
            element->numHeap = NUM;
            element = element->nextSibling;
        }
        return a;
    }
    newHeap = new binomialHeap();
    ptrElement prev = nullptr,
            headA = a->head,
            headB = b->head;
    if (headA == nullptr && headB == nullptr) {
        return nullptr;
    }
    while (headA != nullptr && headB != nullptr) {
        if (headA->degree < headB->degree) {
            ptrElement nextHead = headA->nextSibling;
            if (prev == nullptr) {
                prev = headA;
                newHeap->head = prev;
                newHeap->head->numHeap = NUM;
            } else {
                prev->nextSibling = headA;
                prev = prev->nextSibling;
                prev->numHeap = NUM;
            }
            headA = nextHead;
        } else {
            ptrElement nextHead = headB->nextSibling;
            if (prev == nullptr) {
                prev = headB;
                newHeap->head = prev;
                newHeap->head->numHeap = NUM;
            } else {
                prev->nextSibling = headB;
                prev = prev->nextSibling;
                prev->numHeap = NUM;
            }
            headB = nextHead;
        }
    }
    while (headA != nullptr) {
        ptrElement nextHead = headA->nextSibling;
        if (prev == nullptr) {
            prev = headA;
            newHeap->head = prev;
            newHeap->head->numHeap = NUM;
        } else {
            prev->nextSibling = headA;
            prev = prev->nextSibling;
            prev->numHeap = NUM;
        }
        headA = nextHead;
    }
    while (headB != nullptr) {
        ptrElement nextHead = headB->nextSibling;
        if (prev == nullptr) {
            prev = headB;
            newHeap->head = prev;
            newHeap->head->numHeap = NUM;
        } else {
            prev->nextSibling = headB;
            prev = prev->nextSibling;
            prev->numHeap = NUM;
        }
        headB = nextHead;
    }
    ptrElement curHead = newHeap->head;
    prev = nullptr;
    while (curHead->nextSibling != nullptr) {
        if (curHead->degree == curHead->nextSibling->degree) {
            if ((curHead->key > curHead->nextSibling->key) ||
                ((curHead->key == curHead->nextSibling->key) &&
                 (curHead->index > curHead->nextSibling->index))) {
                ptrElement y = curHead->nextSibling;
                curHead->nextSibling = y->nextSibling;
                y->nextSibling = curHead;
                curHead = y;
                if (prev != nullptr) {
                    prev->nextSibling = y;
                } else {
                    newHeap->head = y;
                }
            }
            ptrElement kid = curHead->child;
            ptrElement y = curHead->nextSibling;
            if (curHead->nextSibling != nullptr) {
                curHead->nextSibling = curHead->nextSibling->nextSibling;
            }
            curHead->degree++;
            if (curHead->child == nullptr) {
                curHead->child = y;
                curHead->child->nextSibling = nullptr;
                curHead->child->p = curHead;
            } else {
                while (kid->nextSibling != nullptr) {
                    kid = kid->nextSibling;
                }
                y->nextSibling = nullptr;
                kid->nextSibling = y;
                kid = y;
                kid->p = curHead;
            }
        } else {
            prev = curHead;
            curHead = curHead->nextSibling;
        }
    }
    return newHeap;
}

void add(int v, int a) {
    newElement = new binomialHeap();
    newElement->head = new binomialHeap::element();
    newElement->head->nextSibling = nullptr;
    newElement->head->degree = 0;
    newElement->head->child = nullptr;
    newElement->head->key = v;
    newElement->head->index = curNumber++;
    newElement->head->p = nullptr;
    newElement->head->numHeap = a;
    heaps[a] = merge(newElement, heaps[a], a);
    elements.push_back(newElement->head);
}

void extractMin(int a) {
    if (heaps[a] == nullptr) {
        return;
    }
    ptrElement element = heaps[a]->head,
            goodElement = element,
            prev = nullptr,
            curPrev = nullptr;
    while (element != nullptr) {
        if ((element->key < goodElement->key) ||
            (element->key == goodElement->key && element->index < goodElement->index)) {
            goodElement = element;
            prev = curPrev;
        }
        curPrev = element;
        element = element->nextSibling;
    }
    if (prev != nullptr) {
        prev->nextSibling = goodElement->nextSibling;
    } else {
        heaps[a]->head = goodElement->nextSibling;
    }
    if (goodElement->child == nullptr) {
        return;
    }
    newHeap = new binomialHeap();
    newHeap->head = goodElement->child;
    prev = newHeap->head;
    while (prev != nullptr) {
        prev->p = nullptr;
        prev->numHeap = goodElement->numHeap;
        prev = prev->nextSibling;
    }
    heaps[a] = merge(heaps[a], newHeap, a);
}

void changeValue(ptrElement element, int value) {
    element->key = value;
    while (element->p != nullptr && ((element->key < element->p->key) ||
                                     ((element->key == element->p->key) &&
                                      (element->index < element->p->index)))) {
        swap(element->key, element->p->key);
        swap(element->index, element->p->index);
        swap(elements[element->index], elements[element->p->index]);
        element = element->p;
    }
    while (true) {
        ptrElement goodElement = element->child,
                checkElement = element->child;
        if (goodElement == nullptr) {
            break;
        }
        while (checkElement != nullptr) {
            if ((checkElement->key < goodElement->key) ||
                (checkElement->key == goodElement->key
                 && checkElement->index < goodElement->index)) {
                goodElement = checkElement;
            }
            checkElement = checkElement->nextSibling;
        }
        if ((goodElement->key < element->key) ||
            (goodElement->key == element->key && goodElement->index < element->index)) {
            swap(element->key, goodElement->key);
            swap(element->index, goodElement->index);
            swap(elements[element->index], elements[goodElement->index]);
            element = goodElement;
        } else {
            break;
        }
    }
}

void deleteElement(ptrElement element) {
    if (element == nullptr) {
        return;
    }
    while (element->p != nullptr) {
        swap(element->key, element->p->key);
        swap(element->index, element->p->index);
        swap(elements[element->index], elements[element->p->index]);
        element = element->p;
    }
    element->key = MIN_VALUE;
    element->index = -228;
    extractMin(element->numHeap);
}

int main() {
    ios_base::sync_with_stdio(false);
    int m;
    cin >> n >> m;
    heaps.resize(n);
    for (int i = 0; i < n; i++) {
        heaps[i] = nullptr;
    }
    while (m--) {
        int type;
        cin >> type;
        if (type == 0) {
            int v, a;
            cin >> a >> v;
            a--;
            add(v, a);
        } else if (type == 1) {
            int a, b;
            cin >> a >> b;
            a--;
            b--;
            heaps[b] = merge(heaps[a], heaps[b], b);
            heaps[a] = nullptr;
        } else if (type == 2) {
            int i;
            cin >> i;
            i--;
            deleteElement(elements[i]);
            elements[i] = nullptr;
        } else if (type == 3) {
            int i, v;
            cin >> i >> v;
            i--;
            changeValue(elements[i], v);
        } else if (type == 4) {
            int a;
            cin >> a;
            a--;
            cout << getMinimum(heaps[a]) << endl;
        } else if (type == 5) {
            int a;
            cin >> a;
            a--;
            extractMin(a);
        } else {
            cout << "Never give up, it's such a wonderful life!" << endl;
        }
    }
    return 0;
}
