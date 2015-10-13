

-- my own implementation of (!!)
-- does not include error handling
elemAt :: [a] -> Int -> a
(x:_) `elemAt` 0  = x
(_:xs) `elemAt` n = xs `elemAt` (n - 1)

-- my own implementation of tail
tail' :: [a] -> [a]
tail' (_:xs) = xs

-- my own implementation of last
last' :: [a] -> a
last' [x]    = x
last' (_:xs) = last' xs
last' []     = error "last': empty list"

-- my own implementation of init
init' :: [a] -> [a]
init' [x]    = []
init' (x:xs) = x:(init' xs)
init' []     = error "init': empty list"

-- my own implementation of length
length' :: [a] -> Int
length' []     = 0
length' (x:xs) = 1 + length' xs

-- my own implementation of null
null' :: [a] -> Bool
null' []    = True
null' (_:_) = False

-- my own implementation of reverse
-- inefficient: traverses list for each (++) append
naive_reverse' :: [a] -> [a]
naive_reverse' []     = []
naive_reverse' (x:xs) = (naive_reverse' xs) ++ [x]

reverse' :: [a] -> [a]
reverse' l = rev l []
    where rev []     rxs = rxs
          rev (x:xs) rxs = rev xs (x:rxs)

-- my own implementation of drop
drop' :: Int -> [a] -> [a]
drop' 0 a      = a
drop' n (x:xs) = drop' (n - 1) xs

-- my own implementation of take
take' :: Int -> [a] -> [a]
take' 0 _      = []
take' n (x:xs) = x : (take' (n - 1) xs)

-- my own implementation of maximum
maximum' :: Ord a => [a] -> a
maximum' [a]    = a
maximum' (x:xs) = max x (maximum' xs)

-- my own implementation of product
product' :: Num a => [a] -> a
product' []     = 1
product' (x:xs) = x * (product' xs)

-- my own implementation of elem
elem' :: Eq a => a -> [a] -> Bool
elem' a [] = False
elem' a (x:xs) = (a == x) || (elem' a xs)

-- my own implementation of cycle
cycle' :: [a] -> [a]
cycle' [] = error "empty list"
cycle' xs = xs' where xs' = xs ++ xs'

-- my own implementation of repeat
repeat' :: a -> [a]
repeat' x = xs where xs = x : xs

-- my own implementation of replicate
replicate' :: Int -> a -> [a]
replicate' n a = take n (repeat' a)


--
-- Jelly Bean Problem
--

--p_r r w = (r + r * w) / (r + w)
--p_w r w = w ** 2 / (r + w)

--pl_r r w = (p_r r w) * (pl_r (r - 1) w) + (p_w r w) * (pl_r r (w - 1))
--pl_r 1 0 = 1.0
--pl_r 0 1 = 0.0
