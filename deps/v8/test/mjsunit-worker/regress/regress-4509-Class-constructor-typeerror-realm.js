"use strict"
// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4509-Class-constructor-typeerror-realm.js");
	}
}
"use strict";
var realm = Realm.create();
var OtherTypeError = Realm.eval(realm, 'TypeError');

class Derived extends Object {
  constructor() {
    return null;
  }
}

assertThrows(() => { new Derived() }, TypeError);

var OtherDerived = Realm.eval(realm,
   "'use strict';" +
   "class Derived extends Object {" +
      "constructor() {" +
        "return null;" +
      "}};");

// Before throwing the TypeError we have to switch to the caller context.
assertThrows(() => { new OtherDerived() }, TypeError);
