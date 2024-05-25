rule Win_Worm_WannaCry_2
{
strings:
	$a0 = { e0c53ad1a4a45482a4a45482a4a45482dfb85882a6a45482cbbb5f82a5a45482 }

condition:
	$a0
}

rule Win_Worm_Whboy_1
{
strings:
	$a0 = { ada132ada12324adad1baeeacdedadcafddad31848cad9183410adad1734cbc140 }

condition:
	$a0
}