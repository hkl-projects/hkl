{-
    Copyright  : Copyright (C) 2014-2020 Synchrotron SOLEIL
                                         L'Orme des Merisiers Saint-Aubin
                                         BP 48 91192 GIF-sur-YVETTE CEDEX
    License    : GPL3+

    Maintainer : Picca Frédéric-Emmanuel <picca@synchrotron-soleil.fr>
    Stability  : Experimental
    Portability: GHC only (not tested)
-}

module Main where

import           Data.Array.Repa.Index             (DIM0)
import           Data.Vector.Storable              (Vector, fromList)
import           Numeric.Units.Dimensional.Prelude (meter, nano, (*~))
import           Pipes
import qualified Pipes.Prelude                     as P

import           Hkl

testSirius :: IO ()
testSirius = runEffect $ fromToPipe 6 from to
             >-> enginesTrajectoryPipe engine
             >-> solveTrajPipe geometry det gaAs
             >-> P.tee P.print
             >-> P.drain
             -- >-> computePipe detector gaAs
             -- >-> P.print
  where
    gaAs :: Sample Cubic
    gaAs = Sample "GaAs" (Cubic (0.56533 *~ nano meter))
           (Parameter "ux" (-90.003382) (Range (-180) 180))
           (Parameter "uy" 0.12907 (Range (-180) 180))
           (Parameter "uz" (-159.91372) (Range (-180) 180))

    geometry :: Geometry
    geometry = Geometry SoleilSiriusKappa (Source (0.1458637 *~ nano meter))
               (fromList [-0.5193202, 64.7853160, 133.5621380, -80.9690000, -0.0223369, 30.0000299])
               (Just [ Parameter "mu" (-0.5193202) (Range (-180) 180)
                     , Parameter "komega" 64.7853160 (Range (-180) 180)
                     , Parameter "kappa" 133.5621380 (Range (-180) 180)
                     , Parameter "kphi" (-80.9690000) (Range (-180) 180)
                     , Parameter "delta" (-0.0223369) (Range (-180) 180)
                     , Parameter "gamma" 30.0000299 (Range (-180) 180)])

    det :: Detector ZeroD DIM0
    det = ZeroD

    engine :: Engine
    engine = Engine "hkl" [ Parameter "h" 0.0 (Range (-1.0) 1.0)
                          , Parameter "k" 0.0 (Range (-1.0) 1.0)
                          , Parameter "l" 2.0 (Range (-1.0) 1.0)
                          ]
             (Mode "bissector_vertical" [])

    from :: Vector Double
    from = fromList [0, 0, 1]

    to :: Vector Double
    to = fromList [0, 0, 6]

test :: IO ()
test = do
  let sample = Sample "test" (Orthorhombic
                              (1.05394 *~ nano meter)
                              (0.25560 *~ nano meter)
                              (1.49050 *~ nano meter))
               (Parameter "ux" (-89.8821) (Range (-180) 180))
               (Parameter "uy" 0.1733 (Range (-180) 180))
               (Parameter "uz" (-84.0081) (Range (-180) 180))

  let geometry = Geometry Uhv (Source (0.0672929 *~ nano meter))
                 (fromList [0.1794, -160.0013, 21.1381, 0.5194])
                 (Just [ Parameter "mu" 0.1794 (Range (-180) 180)
                       , Parameter "omega" (-160.0013) (Range (-180) 180)
                       , Parameter "delta" 21.1381 (Range (-180) 180)
                       , Parameter "gamma" 0.5194 (Range (-180) 180)])
  let det = ZeroD

  -- compute the pseudo axes values
  pseudoAxes <- compute geometry det sample
  print pseudoAxes

  -- solve a pseudo axis problem for the given engine
  let engine = Engine "hkl" [ Parameter "h" 4.0 (Range (-1.0) 1.0)
                            , Parameter "k" 1.0 (Range (-1.0) 1.0)
                            , Parameter "l" 0.3 (Range (-1.0) 1.0)
                            ]
               (Mode "zaxis" [])

  print =<< solve geometry det sample engine

    -- let from = fromList [0, 0, 1 :: Double]
    -- let to = fromList [0, 1, 1 :: Double]
    -- runEffect $ fromToPipe 20 from to
    --           >-> P.print
    -- -- solve a trajectory with Pipes
    -- runEffect $ fromToPipe 10000 from to
    --           >-> enginesTrajectoryPipe engine
    --           >-> solveTrajPipe factory geometry detector sample
    --           >-> P.print
    --           -- >-> P.drain

  return ()

main :: IO ()
main = testSirius
