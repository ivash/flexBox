%downsamples image of size #factor*#targetDimension to size #targetDimension
%therefore #factor must be a positive integer value
classdef downsamplingOperator < basicOperator
    properties
        targetDimension
        factor
        matrix
    end

    methods

        function obj = downsamplingOperator(targetDimension,factor,varargin)
            if (nargin > 2 && numel(varargin) == 1)
                varargin = varargin{1};
            end
            vararginParser;

            obj.targetDimension = targetDimension;
            obj.factor = factor;

            obj.matrix = generateDownsamplingMatrixNew( targetDimension,factor*targetDimension );
        end

        function result = mtimes(obj,vector)
            result = obj.matrix * vector;
            
            if (obj.isMinus)
                result = -result;
            end
        end

        function result = abs(obj)
            result = abs(obj.matrix);
        end

        function mat = returnMatrix(obj)
            mat = obj.matrix;
        end

        function res = ctranspose(obj)
            res = obj;
            res.matrix = res.matrix';
        end

        function result = size(obj,varargin)
            if (nargin < 2)
                result = size(obj.matrix);
            else
                result = size(obj.matrix,varargin{1});
            end
        end

        function result = getMaxRowSumAbs(obj)
            result = 1;
        end
    end

end
