// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-513602.js");
	}
}
function Parent() {}

function Child() {}
Child.prototype = new Parent();
var child = new Child();

function crash() {
  return child.__proto__;
}

crash();
crash();

// Trigger a fast->slow->fast dance of Parent.prototype's map...
Parent.prototype.__defineSetter__("foo", function() { print("A"); });
Parent.prototype.__defineSetter__("foo", function() { print("B"); });
// ...and collect more type feedback.
crash();

// Now modify the prototype chain. The right cell fails to get invalidated.
delete Object.prototype.__proto__;
crash();
