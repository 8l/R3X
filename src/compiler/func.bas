#! No global variables yet :(
function x, 3
	print "I'm a function take takes 3 args"
	let a = $1
	print "the number got in the first arg is: "; a
	let b = $2+15
	print "the number got in the second arg + 15 is :"; b
	let f = 5
	print "f in this scope is: "; f
	return $3
endf
function add2numbers, 2
	let a = $1+$2
	return a
endf
function subtract2numbers, 2
	let a = $1-$2
	return a
endf
function misc, 0
	print "f in this scope is"; f
	#! Dont FORGET TO RETURN! else cause undefined behaviour lol
	return 0
endf
function main, 0
	let f = 1
	@misc()
	let c = 24
	let retval = @x(100, 200, c * 5 + 2)
	print "return value: "; retval
	if retval = c*5+2 goto correct
	print "lol, this failed."
	end
:correct
	print "lol it's right"
	print "f in this scope is: "; f
	print "adding 2 numbers: 57 and 90"
	let result = @add2numbers(57, 90)
	print "result = "; result
	print "subtracting 2 numbers: 30 and 10"
	let result = @subtract2numbers(30, 10)
	print "result = "; result
	end
endf
