// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --predictable

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-6210.js");
	}
}
const str = '2016-01-02';

function testToUint32InSplit() {
  var re;
  function toDictMode() {
    re.x = 42;
    delete re.x;
    return "def";
  }

  re = /./g;  // Needs to be global to trigger lastIndex accesses.
  return re[Symbol.replace]("abc", { valueOf: toDictMode });
}

function testToStringInReplace() {
  var re;
  function toDictMode() {
    re.x = 42;
    delete re.x;
    return 42;
  }

  re = /./g;  // Needs to be global to trigger lastIndex accesses.
  return re[Symbol.split]("abc", { valueOf: toDictMode });
}

testToUint32InSplit();
testToStringInReplace();
