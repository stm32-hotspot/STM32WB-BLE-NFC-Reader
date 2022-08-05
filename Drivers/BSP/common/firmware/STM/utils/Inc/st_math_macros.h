/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/
/*
 *      PROJECT:   ASxxxx firmware
 *      $Revision: $
 *      LANGUAGE:  ANSI C
 */

/*! \file
 *
 *  \author R. Veigl
 *
 *  \brief basic mathematic macros
 *
 */

#ifndef ST_MATH_MACROS_H
#define ST_MATH_MACROS_H


#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

#define DIV_ROUND_UP(a, b) (((a) + (b) - 1) / (b))
#define DIV_ROUND_DOWN(a, b) ((a) / (b))
#define DIV_ROUND_NEAREST(a, b) (((a) + ((b)/2) - 1) / (b))


#endif /* ST_MATH_MACROS_H */
