print("C Worker start...");
var Max_count =100000;
var count = 0;
var sab;
var array;
var c;

const kLockStateLocked = 1;
const kLockStateUnlocked = 0;
const kLockStateLockedAndParked = 2;

// slow lock
function lockObjectSlow(sab) {
  var int32a = new Int32Array(sab);
  var desiredState = kLockStateLocked;
    for (;;) {
        if (Atomics.compareExchange(int32a, 2, kLockStateUnlocked, desiredState) == kLockStateUnlocked)
            return;
        desiredState = kLockStateLockedAndParked;
        Atomics.compareExchange(int32a, 2, kLockStateLocked, kLockStateLockedAndParked);
        Atomics.wait(int32a, 2, kLockStateLockedAndParked);
    }
}



// lock
// function lockObject(sab) {
//   var int32a = new Int32Array(sab);
//   if ((c = Atomics.compareExchange(int32a, 2, 0, 1)) !== 0) { // 不为0，说明其他人持锁
//     do {
//          // 如果依旧得不到锁
//       if (c === 2 || Atomics.compareExchange(int32a, 2, 1, 2) != 0) {
//         Atomics.wait(int32a, 2, 2); // 暂停
//       }
//       // 再次尝试获取锁
//     } while ((c = Atomics.compareExchange(int32a, 2, 0, 2)) !== 0)
//   }
// }

function lockObject(sab) {
  var int32a = new Int32Array(sab);
  if (Atomics.compareExchange(int32a, 2, kLockStateUnlocked, kLockStateLocked) == kLockStateUnlocked)
    return;
  lockObjectSlow(sab);
}

//unlock
// function unlockObject(sab) {
//   var int32a = new Int32Array(sab);
//   let v0 = Atomics.sub(int32a, 2, 1);
//   // 此时拥有锁，状态为1或2
//   if (v0 != 1) {
//     Atomics.store(int32a, 2, 0); // 释放锁
//     Atomics.wake(int32a, 2, 1); // 唤醒一个 wait 的
//   }
// }

function unlockObject(sab) {
  var int32a = new Int32Array(sab);
  if (Atomics.exchange(int32a, 2, kLockStateUnlocked) == kLockStateLocked)
     return;
  Atomics.wake(int32a, 2, 1);
}


onmessage = function(env) {
    var data = env.data;
    print("C Worker got a message");
    var sab = data.heap;
    var array = new Uint32Array(sab);
    for (let i = 0; i < 100000000; i++) {
      lockObject(sab);
      unlockObject(sab);
    }
}
