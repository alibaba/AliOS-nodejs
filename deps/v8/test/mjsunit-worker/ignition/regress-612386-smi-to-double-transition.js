// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --no-inline-new

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/ignition/regress-612386-smi-to-double-transition.js");
	}
}
function keyed_store(obj, key, value) {
  obj[key] = value;
}

function foo() {
  obj = {};
  obj.smi = 1;
  obj.dbl = 1.5;
  obj.obj = {a:1};

  // Transition keyed store IC to polymorphic.
  keyed_store(obj, "smi", 100);
  keyed_store(obj, "dbl", 100);
  keyed_store(obj, "obj", 100);

  // Now call with a PACKED_SMI_ELEMENTS object.
  var smi_array = [5, 1, 1];
  keyed_store(smi_array, 1, 6);
  // Transition from PACKED_SMI_ELEMENTS to PACKED_DOUBLE_ELEMENTS.
  keyed_store(smi_array, 2, 1.2);
}

foo();
