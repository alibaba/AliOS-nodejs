print("P Worker start...");
var Max_count =100000;
var count = 0;
var sab;
var array;

var c;

const kLockStateLocked = 1;
const kLockStateUnlocked = 0;
const kLockStateLockedAndParked = 2;


// slow lock
function lockObjectSlow(sab, int32a) {
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
function lockObject(sab) {
  var int32a = new Int32Array(sab);
  if (Atomics.compareExchange(int32a, 2, kLockStateUnlocked, kLockStateLocked) == kLockStateUnlocked)
    return;
  lockObjectSlow(sab, int32a);
}

//unlock
function unlockObject(sab) {
  var int32a = new Int32Array(sab);
  if (Atomics.exchange(int32a, 2, kLockStateUnlocked) == kLockStateLocked)
     return;
  Atomics.wake(int32a, 2, 1);
}


function sleep(sec) {
  var count = 0;
  for (var i = 0;i<sec;i++) {
    for(var j = 0 ;j<sec;j++) {
      count = i +j;
    }
  }
  return count;
}

onmessage = function(env) {
    var data = env.data;
    print("P Worker got a message");
    var sab = data.heap;
    var array = new Uint32Array(sab);
    array[0] = 2000;
    array[1] = 1000;
    var length = array.length;
    var index = 0;
    for(let i = 0; i < 10000000; i++) {
      lockObject(sab);
      for(i = 0;i<length;i++) {
        array[i] = index;
      }
      if(index++ == 0xFFFFFFFF) {
         index = 0;
      }
      unlockObject(sab);
    }

}
